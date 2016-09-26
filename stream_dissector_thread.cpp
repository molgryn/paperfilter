#include <v8.h>
#include <nan.h>
#include <v8pp/class.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <cstdlib>
#include "nylon_context.hpp"
#include "stream_chunk.hpp"
#include "stream_dissector_thread.hpp"
#include "layer.hpp"
#include "packet.hpp"

using namespace v8;

namespace {
class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
  ArrayBufferAllocator() {}
  ~ArrayBufferAllocator() {}

  virtual void *Allocate(size_t size) { return malloc(size); }
  virtual void *AllocateUninitialized(size_t size) { return malloc(size); }
  virtual void Free(void *data, size_t) { free(data); }
};

struct DissectorFunc {
  std::vector<std::string> namespaces;
  std::vector<std::regex> regexNamespaces;
  v8::UniquePersistent<v8::Function> func;
};
}

class StreamDissectorThread::Private {
public:
  Private(const std::shared_ptr<Context> &ctx);
  ~Private();
  const std::vector<const DissectorFunc *> &
  findDessector(const std::string &ns);

public:
  std::thread thread;
  std::mutex mutex;
  std::condition_variable cond;
  std::queue<std::unique_ptr<StreamChunk>> chunks;
  bool closed = false;

  std::shared_ptr<Context> ctx;
  std::vector<DissectorFunc> dissectors;
  std::unordered_map<std::string, std::vector<const DissectorFunc *>> nsMap;
};

StreamDissectorThread::Private::Private(const std::shared_ptr<Context> &ctx)
    : ctx(ctx) {

  thread = std::thread([this]() {
    Context &ctx = *this->ctx;
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = new ArrayBufferAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);

    {
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Context> context = v8::Context::New(isolate);
      v8::Context::Scope context_scope(context);
      v8::TryCatch try_catch;
      NylonContext::init(isolate);

      for (const Dissector &diss : ctx.dissectors) {
        v8::Local<v8::Context> moduleContext = v8::Context::New(isolate);
        v8::Context::Scope context_scope(moduleContext);
        NylonContext::init(isolate);

        v8::Local<v8::Object> moduleObj = v8::Object::New(isolate);
        moduleContext->Global()->Set(v8::String::NewFromUtf8(isolate, "module"),
                                     moduleObj);

        v8::Local<v8::Function> func;
        Nan::MaybeLocal<Nan::BoundScript> script =
            Nan::CompileScript(v8pp::to_v8(isolate, diss.script));
        if (!script.IsEmpty()) {
          Nan::RunScript(script.ToLocalChecked());
          v8::Local<v8::Value> result =
              moduleObj->Get(v8::String::NewFromUtf8(isolate, "exports"));

          if (!result.IsEmpty() && result->IsFunction()) {
            func = result.As<v8::Function>();
          }
        }
        if (func.IsEmpty()) {
          if (ctx.errorCb) {
            Nan::Utf8String str(try_catch.Exception());
            if (*str)
              ctx.errorCb(*str);
          }
        } else {
          dissectors.push_back(
              {diss.namespaces, diss.regexNamespaces,
               v8::UniquePersistent<v8::Function>(isolate, func)});
        }
      }

      std::unordered_map<
          std::string, std::vector<v8::UniquePersistent<v8::Object>>> instances;

      while (true) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this] { return !chunks.empty() || closed; });
        if (closed)
          break;

        std::unique_ptr<StreamChunk> chunk = std::move(chunks.front());
        chunks.pop();
        lock.unlock();

        const std::string &key = chunk->ns() + "@" + chunk->id();
        auto it = instances.find(key);
        if (it == instances.end()) {
          std::vector<v8::UniquePersistent<v8::Object>> objs;
          const auto &dissectors = findDessector(chunk->ns());
          for (const DissectorFunc *diss : dissectors) {
            v8::Local<v8::Function> func =
                v8::Local<v8::Function>::New(isolate, diss->func);
            v8::Local<v8::Object> obj = func->NewInstance();
            if (obj.IsEmpty()) {
              if (ctx.errorCb) {
                Nan::Utf8String str(try_catch.Exception());
                if (*str)
                  ctx.errorCb(*str);
              }
            } else {
              objs.push_back(v8::UniquePersistent<v8::Object>(isolate, obj));
            }
          }
          it = instances.insert(std::make_pair(key, std::move(objs))).first;
        }

        const std::vector<v8::UniquePersistent<v8::Object>> &objs = it->second;
        std::shared_ptr<Layer> layer = chunk->layer();
        std::shared_ptr<Packet> packet = layer->packet();
        v8::Local<v8::Object> layerObj =
            v8pp::class_<Layer>::reference_external(isolate, layer.get());
        v8::Local<v8::Object> packetObj =
            v8pp::class_<Packet>::reference_external(isolate, packet.get());

        for (const auto &unique : objs) {
          v8::Local<v8::Object> obj =
              v8::Local<v8::Object>::New(isolate, unique);
          v8::Local<v8::Value> analyze =
              obj->Get(v8pp::to_v8(isolate, "analyze"));
          if (!analyze.IsEmpty() && analyze->IsFunction()) {
            v8::Local<v8::Function> analyzeFunc = analyze.As<v8::Function>();
            v8::Handle<v8::Value> args[2] = {packetObj, layerObj};
            v8::Local<v8::Value> result = analyzeFunc->Call(
                isolate->GetCurrentContext()->Global(), 2, args);

            if (result.IsEmpty()) {
              if (ctx.errorCb) {
                Nan::Utf8String str(try_catch.Exception());
                if (*str)
                  ctx.errorCb(*str);
              }
            } else {

              printf("%s %p %p %p\n", chunk->id().c_str(), chunk->layer().get(),
                     chunk->layer()->packet().get(), &objs);
            }
          }
        }

        v8pp::class_<Packet>::unreference_external(isolate, packet.get());
        v8pp::class_<Layer>::unreference_external(isolate, layer.get());

        lock.lock();
      }
    }

    isolate->Dispose();
  });
}

StreamDissectorThread::Private::~Private() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    closed = true;
  }
  cond.notify_one();
  if (thread.joinable())
    thread.join();
}

const std::vector<const DissectorFunc *> &
StreamDissectorThread::Private::findDessector(const std::string &ns) {
  static const std::vector<DissectorFunc *> null;

  auto it = nsMap.find(ns);
  if (it != nsMap.end())
    return it->second;

  std::vector<const DissectorFunc *> &funcs = nsMap[ns];
  for (const auto &diss : dissectors) {
    for (const std::string &dissNs : diss.namespaces) {
      if (dissNs == ns) {
        funcs.push_back(&diss);
        break;
      }
    }
    if (funcs.empty()) {
      for (const std::regex &regex : diss.regexNamespaces) {
        if (std::regex_match(ns, regex)) {
          funcs.push_back(&diss);
          break;
        }
      }
    }
  }

  return funcs;
}

StreamDissectorThread::StreamDissectorThread(
    const std::shared_ptr<Context> &ctx)
    : d(new Private(ctx)) {}

StreamDissectorThread::~StreamDissectorThread() {}

void StreamDissectorThread::insert(std::unique_ptr<StreamChunk> chunk) {
  std::lock_guard<std::mutex> lock(d->mutex);
  d->chunks.push(std::move(chunk));
  d->cond.notify_one();
}

void StreamDissectorThread::clearStream(const std::string &ns,
                                        const std::string &id) {
  std::lock_guard<std::mutex> lock(d->mutex);
}

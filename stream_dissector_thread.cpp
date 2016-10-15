#include "stream_dissector_thread.hpp"
#include "layer.hpp"
#include "packet.hpp"
#include "paper_context.hpp"
#include "stream_chunk.hpp"
#include "virtual_packet.hpp"
#include "error.hpp"
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <nan.h>
#include <queue>
#include <thread>
#include <unordered_map>
#include <v8.h>
#include <v8pp/class.hpp>

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

    // workaround for chromium task runner
    char dummyData[16] = {0};
    isolate->SetData(0, dummyData);

    {
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Context> context = v8::Context::New(isolate);
      v8::Context::Scope context_scope(context);
      v8::TryCatch try_catch;
      PaperContext::init(isolate);

      for (const Dissector &diss : ctx.dissectors) {
        v8::Local<v8::Object> moduleObj = v8::Object::New(isolate);
        context->Global()->Set(v8::String::NewFromUtf8(isolate, "module"),
                               moduleObj);

        v8::Local<v8::Function> func;
        Nan::MaybeLocal<Nan::BoundScript> script = Nan::CompileScript(
            v8pp::to_v8(isolate, "(function(){" + diss.script + "})()"),
            v8::ScriptOrigin(v8pp::to_v8(isolate, diss.resourceName)));
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
            ctx.errorCb(messageToJson(try_catch.Message()));
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
                ctx.errorCb(messageToJson(try_catch.Message()));
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
        v8::Local<v8::Object> chunkObj =
            v8pp::class_<StreamChunk>::import_external(isolate,
                                                       new StreamChunk(*chunk));

        std::vector<std::unique_ptr<VirtualPacket>> vpackets;
        std::vector<std::unique_ptr<StreamChunk>> streams;

        for (const auto &unique : objs) {
          v8::Local<v8::Object> obj =
              v8::Local<v8::Object>::New(isolate, unique);
          v8::Local<v8::Value> analyze =
              obj->Get(v8pp::to_v8(isolate, "analyze"));
          if (!analyze.IsEmpty() && analyze->IsFunction()) {
            v8::Local<v8::Function> analyzeFunc = analyze.As<v8::Function>();
            v8::Handle<v8::Value> args[3] = {packetObj, layerObj, chunkObj};
            v8::Local<v8::Value> result = analyzeFunc->Call(obj, 3, args);

            if (result.IsEmpty()) {
              if (ctx.errorCb) {
                ctx.errorCb(messageToJson(try_catch.Message()));
              }
            } else if (result->IsArray()) {
              v8::Local<v8::Array> array = result.As<v8::Array>();
              for (uint32_t i = 0; i < array->Length(); ++i) {
                if (VirtualPacket *vp =
                        v8pp::class_<VirtualPacket>::unwrap_object(
                            isolate, array->Get(i))) {
                  vpackets.push_back(
                      std::unique_ptr<VirtualPacket>(new VirtualPacket(*vp)));
                } else if (StreamChunk *stream =
                               v8pp::class_<StreamChunk>::unwrap_object(
                                   isolate, array->Get(i))) {

                  streams.push_back(std::unique_ptr<StreamChunk>(
                      new StreamChunk(*stream, chunk->layer())));
                }
              }
            } else if (VirtualPacket *vp =
                           v8pp::class_<VirtualPacket>::unwrap_object(isolate,
                                                                      result)) {
              vpackets.push_back(
                  std::unique_ptr<VirtualPacket>(new VirtualPacket(*vp)));
            } else if (StreamChunk *stream =
                           v8pp::class_<StreamChunk>::unwrap_object(isolate,
                                                                    result)) {

              streams.push_back(std::unique_ptr<StreamChunk>(
                  new StreamChunk(*stream, chunk->layer())));
            }
          }
        }

        v8pp::class_<Packet>::unreference_external(isolate, packet.get());
        v8pp::class_<Layer>::unreference_external(isolate, layer.get());

        if (ctx.vpacketsCb)
          ctx.vpacketsCb(std::move(vpackets));

        if (ctx.streamsCb)
          ctx.streamsCb(std::move(streams));

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

#include "session.hpp"
#include "buffer.hpp"
#include "dissector.hpp"
#include "dissector_thread.hpp"
#include "filter_thread.hpp"
#include "layer.hpp"
#include "packet.hpp"
#include "packet_queue.hpp"
#include "packet_store.hpp"
#include "pcap.hpp"
#include "permission.hpp"
#include "stream_chunk.hpp"
#include "stream_dispatcher.hpp"
#include "virtual_packet.hpp"
#include <nan.h>
#include <unordered_set>
#include <uv.h>
#include <v8pp/class.hpp>
#include <v8pp/object.hpp>

using namespace v8;

struct FilterContext {
  std::vector<std::unique_ptr<FilterThread>> threads;
  std::shared_ptr<FilterThread::Context> ctx;
};

class Session::Private {
public:
  Private();
  ~Private();
  void error(const std::string &err);

public:
  PacketQueue queue;
  PacketStore store;
  std::vector<std::unique_ptr<DissectorThread>> dissectorThreads;
  std::unordered_map<std::string, FilterContext> filterThreads;
  std::string ns;
  std::string filterScript;

  UniquePersistent<Function> packetsCb;
  UniquePersistent<Function> filtersCb;
  UniquePersistent<Function> errorCb;
  uv_async_t packetsCbAsync;
  uv_async_t filtersCbAsync;
  uv_async_t errorCbAsync;

  std::unique_ptr<StreamDispatcher> streamDispatcher;
  std::unique_ptr<Pcap> pcap;

  std::mutex errorMutex;
  std::unordered_set<std::string> recentErrors;
};

Session::Private::Private() {
  errorCbAsync.data = this;
  uv_async_init(uv_default_loop(), &errorCbAsync, [](uv_async_t *handle) {
    Session::Private *d = static_cast<Session::Private *>(handle->data);
    if (!d->errorCb.IsEmpty()) {
      std::unordered_set<std::string> errs;
      {
        std::lock_guard<std::mutex> lock(d->errorMutex);
        errs.swap(d->recentErrors);
      }
      Isolate *isolate = Isolate::GetCurrent();
      for (const std::string &err : errs) {
        Handle<Value> args[1] = {v8pp::to_v8(isolate, err)};
        Local<Function> func = Local<Function>::New(isolate, d->errorCb);
        func->Call(isolate->GetCurrentContext()->Global(), 1, args);
      }
    }
  });

  packetsCbAsync.data = this;
  uv_async_init(uv_default_loop(), &packetsCbAsync, [](uv_async_t *handle) {
    Session::Private *d = static_cast<Session::Private *>(handle->data);
    if (!d->packetsCb.IsEmpty()) {
      Isolate *isolate = Isolate::GetCurrent();
      Handle<Value> args[1] = {Number::New(isolate, d->store.maxSeq())};
      Local<Function> func = Local<Function>::New(isolate, d->packetsCb);
      func->Call(isolate->GetCurrentContext()->Global(), 1, args);
    }
  });

  filtersCbAsync.data = this;
  uv_async_init(uv_default_loop(), &filtersCbAsync, [](uv_async_t *handle) {
    Session::Private *d = static_cast<Session::Private *>(handle->data);
    if (!d->filtersCb.IsEmpty()) {
      for (const auto &pair : d->filterThreads) {
        std::vector<uint32_t> packets;
        {
          std::lock_guard<std::mutex>(pair.second.ctx->mutex);
          pair.second.ctx->filterdPackets.swap(packets);
        }
        std::sort(packets.begin(), packets.end());

        Isolate *isolate = Isolate::GetCurrent();
        Local<Array> array = Array::New(isolate, packets.size());
        for (size_t i = 0; i < packets.size(); ++i) {
          array->Set(i, v8pp::to_v8(isolate, packets[i]));
        }

        Handle<Value> args[2] = {v8pp::to_v8(isolate, pair.first), array};
        Local<Function> func = Local<Function>::New(isolate, d->filtersCb);
        func->Call(isolate->GetCurrentContext()->Global(), 2, args);
      }
    }
  });

  auto storeCb = [this](uint32_t maxSeq) { uv_async_send(&packetsCbAsync); };
  store.addHandler(storeCb);
}

void Session::Private::error(const std::string &err) {
  {
    std::lock_guard<std::mutex> lock(errorMutex);
    recentErrors.insert(err);
  }
  uv_async_send(&errorCbAsync);
}

Session::Private::~Private() { queue.close(); }

Session::Session(v8::Local<v8::Value> option) : d(new Private()) {
  if (option.IsEmpty() || !option->IsObject())
    return;

  Isolate *isolate = Isolate::GetCurrent();
  Local<Object> opt = option.As<Object>();
  v8pp::get_option(isolate, opt, "namespace", d->ns);
  v8pp::get_option(isolate, opt, "filterScript", d->filterScript);

  Local<Array> dissectorArray;
  std::vector<Dissector> dissectors;
  if (v8pp::get_option(isolate, opt, "dissectors", dissectorArray)) {
    for (uint32_t i = 0; i < dissectorArray->Length(); ++i) {
      Local<Value> diss = dissectorArray->Get(i);
      if (!diss.IsEmpty() && diss->IsObject()) {
        dissectors.emplace_back(diss.As<Object>());
      }
    }
  }

  Local<Array> streamDissectorArray;
  std::vector<Dissector> streamDissectors;
  if (v8pp::get_option(isolate, opt, "stream_dissectors",
                       streamDissectorArray)) {
    for (uint32_t i = 0; i < streamDissectorArray->Length(); ++i) {
      Local<Value> diss = streamDissectorArray->Get(i);
      if (!diss.IsEmpty() && diss->IsObject()) {
        streamDissectors.emplace_back(diss.As<Object>());
      }
    }
  }

  auto dissCtx = std::make_shared<DissectorThread::Context>();
  dissCtx->queue = &d->queue;
  dissCtx->packetCb = [this](const std::shared_ptr<Packet> &pkt) {
    d->store.insert(pkt);
  };
  dissCtx->streamsCb =
      [this](uint32_t seq, std::vector<std::unique_ptr<StreamChunk>> streams) {
        d->streamDispatcher->insert(seq, std::move(streams));
      };
  dissCtx->dissectors.swap(dissectors);
  dissCtx->errorCb =
      std::bind(&Private::error, std::ref(d), std::placeholders::_1);

  for (int i = 0; i < 4; ++i) {
    d->dissectorThreads.emplace_back(new DissectorThread(dissCtx));
  }

  auto streamCtx = std::make_shared<StreamDispatcher::Context>();
  streamCtx->dissectors.swap(streamDissectors);
  streamCtx->errorCb =
      std::bind(&Private::error, std::ref(d), std::placeholders::_1);
  streamCtx->streamsCb =
      [this](std::vector<std::unique_ptr<StreamChunk>> streams) {
        d->streamDispatcher->insert(std::move(streams));
      };
  streamCtx->vpacketsCb =
      [this](std::vector<std::unique_ptr<VirtualPacket>> vpackets) {
        for (auto &vp : vpackets) {
          d->queue.push(std::unique_ptr<Packet>(new Packet(*vp)));
        }
      };
  d->streamDispatcher.reset(new StreamDispatcher(streamCtx));

  auto pcapCtx = std::make_shared<Pcap::Context>();
  pcapCtx->errorCb =
      std::bind(&Private::error, std::ref(d), std::placeholders::_1);
  pcapCtx->packetCb = [this](std::unique_ptr<Packet> pkt) {
    analyze(std::move(pkt));
  };
  d->pcap.reset(new Pcap(pcapCtx));
}

Session::~Session() {}

void Session::analyze(std::unique_ptr<Packet> pkt) {
  const auto &layer = std::make_shared<Layer>(d->ns);
  layer->setName("Raw Layer");
  layer->setPayload(pkt->payload());
  pkt->addLayer(layer);
  d->queue.push(std::move(pkt));
}

void Session::filter(const std::string &name, const std::string &filter) {
  d->filterThreads.erase(name);

  if (!filter.empty()) {
    FilterContext &context = d->filterThreads[name];
    context.ctx = std::make_shared<FilterThread::Context>();
    context.ctx->store = &d->store;
    context.ctx->filter = filter;
    context.ctx->script = d->filterScript;
    context.ctx->updateCb = [this](uint32_t seq) {
      uv_async_send(&d->filtersCbAsync);
    };
    context.ctx->errorCb =
        std::bind(&Private::error, std::ref(d), std::placeholders::_1);
    for (int i = 0; i < 4; ++i) {
      context.threads.emplace_back(new FilterThread(context.ctx));
    }
  }
}

v8::Local<v8::Function> Session::errorCallback() const {
  return Local<Function>::New(Isolate::GetCurrent(), d->errorCb);
}

void Session::setErrorCallback(const v8::Local<v8::Function> &cb) {
  d->errorCb.Reset(Isolate::GetCurrent(), cb);
}

Local<Function> Session::packetCallback() const {
  return Local<Function>::New(Isolate::GetCurrent(), d->packetsCb);
}

void Session::setPacketCallback(const Local<Function> &cb) {
  d->packetsCb.Reset(Isolate::GetCurrent(), cb);
}

v8::Local<v8::Function> Session::filterCallback() const {
  return Local<Function>::New(Isolate::GetCurrent(), d->filtersCb);
}
void Session::setFilterCallback(const v8::Local<v8::Function> &cb) {
  d->filtersCb.Reset(Isolate::GetCurrent(), cb);
}

std::shared_ptr<const Packet> Session::get(uint32_t seq) const {
  return d->store.get(seq);
}

std::string Session::ns() const { return d->ns; }

bool Session::permission() const { return Permission::test(); }

v8::Local<v8::Array> Session::devices() const {
  Isolate *isolate = Isolate::GetCurrent();
  const std::vector<Pcap::Device> &devs = d->pcap->devices();
  v8::Local<v8::Array> array = v8::Array::New(isolate, devs.size());
  for (size_t i = 0; i < devs.size(); ++i) {
    const Pcap::Device &dev = devs[i];
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    v8pp::set_option(isolate, obj, "id", dev.id);
    v8pp::set_option(isolate, obj, "name", dev.name);
    v8pp::set_option(isolate, obj, "description", dev.description);
    v8pp::set_option(isolate, obj, "link", dev.link);
    v8pp::set_option(isolate, obj, "loopback", dev.loopback);
    array->Set(i, obj);
  }
  return array;
}

void Session::setInterface(const std::string &ifs) {
  d->pcap->setInterface(ifs);
}
std::string Session::interface() const { return d->pcap->interface(); }
void Session::setPromiscuous(bool promisc) { d->pcap->setPromiscuous(promisc); }
bool Session::promiscuous() const { return d->pcap->promiscuous(); }
void Session::setSnaplen(int len) { d->pcap->setSnaplen(len); }
int Session::snaplen() const { return d->pcap->snaplen(); }
bool Session::setBPF(const std::string &filter, std::string *error) {
  return d->pcap->setBPF(filter, error);
}

void Session::start() { d->pcap->start(); }
void Session::stop() { d->pcap->stop(); }

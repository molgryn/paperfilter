#include "packet.hpp"
#include "buffer.hpp"
#include "layer.hpp"
#include <node_buffer.h>
#include <v8pp/class.hpp>
#include <v8pp/object.hpp>

using namespace v8;

class Packet::Private {
public:
  Private();
  ~Private();

public:
  uint32_t seq = 0;
  uint32_t ts_sec = 0;
  uint32_t ts_nsec = 0;
  uint32_t length = 0;
  std::string summary;
  std::string extension;
  std::shared_ptr<const std::vector<char>> payload;
  std::unordered_map<std::string, std::shared_ptr<Layer>> layers;
};

Packet::Private::Private() {}

Packet::Private::~Private() {}

Packet::Packet(v8::Local<v8::Object> option) : d(new Private()) {
  Isolate *isolate = Isolate::GetCurrent();
  v8pp::get_option(isolate, option, "ts_sec", d->ts_sec);
  v8pp::get_option(isolate, option, "ts_nsec", d->ts_nsec);
  v8pp::get_option(isolate, option, "summary", d->summary);
  v8pp::get_option(isolate, option, "extension", d->extension);
  v8pp::get_option(isolate, option, "length", d->length);
  Local<Value> payload = option->Get(v8pp::to_v8(isolate, "payload"));
  if (node::Buffer::HasInstance(payload)) {
    auto buffer = std::make_shared<std::vector<char>>();
    buffer->assign(node::Buffer::Data(payload),
                   node::Buffer::Data(payload) + node::Buffer::Length(payload));
    d->payload = buffer;
  }
}

Packet::~Packet() {}

uint32_t Packet::seq() const { return d->seq; }

void Packet::setSeq(uint32_t id) { d->seq = id; }

uint32_t Packet::ts_sec() const { return d->ts_sec; }

uint32_t Packet::ts_nsec() const { return d->ts_nsec; }

std::string Packet::summary() const { return d->summary; }

std::string Packet::extension() const { return d->extension; }

uint32_t Packet::length() const { return d->length; }

std::shared_ptr<const std::vector<char>> Packet::payload() const {
  return d->payload;
}

v8::Local<v8::Object> Packet::payloadBuffer() const {
  Isolate *isolate = Isolate::GetCurrent();
  return v8pp::class_<Buffer>::create_object(isolate, d->payload);
}

void Packet::addLayer(const std::shared_ptr<Layer> &layer) {
  d->layers[layer->ns()] = layer;
}

const std::unordered_map<std::string, std::shared_ptr<Layer>> &
Packet::layers() const {
  return d->layers;
}

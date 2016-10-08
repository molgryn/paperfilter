#include "packet.hpp"
#include "buffer.hpp"
#include "layer.hpp"
#include "virtual_packet.hpp"
#include <chrono>
#include <node_buffer.h>
#include <v8pp/class.hpp>
#include <v8pp/object.hpp>
#include <pcap.h>

using namespace v8;

class Packet::Private {
public:
  Private();
  ~Private();

public:
  uint32_t seq = 0;
  uint32_t ts_sec = std::chrono::seconds(std::time(NULL)).count();
  uint32_t ts_nsec = 0;
  uint32_t length = 0;
  std::string summary;
  std::string extension;
  std::unique_ptr<Buffer> payload;
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
    d->payload.reset(new Buffer(buffer));
    d->payload->freeze();
  }
}

Packet::Packet(const VirtualPacket &vp) : d(new Private()) {
  d->payload = vp.payload();
  d->payload->freeze();
  d->length = 0;
  addLayer(std::make_shared<Layer>(vp.ns()));
}

Packet::Packet(const struct pcap_pkthdr *h, const uint8_t *bytes) : d(new Private())
{
  d->ts_sec = h->ts.tv_sec;
  d->ts_nsec = h->ts.tv_usec;
  d->length = h->len;
  auto buffer = std::make_shared<std::vector<char>>();
  buffer->assign(bytes, bytes + h->caplen);
  d->payload.reset(new Buffer(buffer));
  d->payload->freeze();
}

Packet::~Packet() {}

uint32_t Packet::seq() const { return d->seq; }

void Packet::setSeq(uint32_t id) { d->seq = id; }

uint32_t Packet::ts_sec() const { return d->ts_sec; }

uint32_t Packet::ts_nsec() const { return d->ts_nsec; }

std::string Packet::summary() const { return d->summary; }

std::string Packet::extension() const { return d->extension; }

uint32_t Packet::length() const { return d->length; }

std::unique_ptr<Buffer> Packet::payload() const { return d->payload->slice(); }

v8::Local<v8::Object> Packet::payloadBuffer() const {
  Isolate *isolate = Isolate::GetCurrent();
  return v8pp::class_<Buffer>::import_external(isolate,
                                               d->payload->slice().release());
}

void Packet::addLayer(const std::shared_ptr<Layer> &layer) {
  d->layers[layer->ns()] = layer;
}

const std::unordered_map<std::string, std::shared_ptr<Layer>> &
Packet::layers() const {
  return d->layers;
}

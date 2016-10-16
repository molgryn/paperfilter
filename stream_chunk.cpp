#include "stream_chunk.hpp"
#include "buffer.hpp"
#include "layer.hpp"
#include <v8pp/class.hpp>

using namespace v8;

class StreamChunk::Private {
public:
  std::string ns;
  std::string id;
  std::shared_ptr<Layer> layer;
  std::unique_ptr<Buffer> payload = std::unique_ptr<Buffer>(new Buffer());
  bool end = false;
};

StreamChunk::StreamChunk(const std::string &ns, const std::string &id,
                         v8::Local<v8::Object> obj)
    : d(std::make_shared<Private>()) {
  d->ns = ns;
  d->id = id;
  if (Layer *layer =
          v8pp::class_<Layer>::unwrap_object(v8::Isolate::GetCurrent(), obj)) {
    d->layer = std::make_shared<Layer>(*layer);
  }
}

StreamChunk::StreamChunk(const StreamChunk &stream) : d(stream.d) {}

StreamChunk::~StreamChunk() {}

std::string StreamChunk::ns() const { return d->ns; }

std::string StreamChunk::id() const { return d->id; }

std::shared_ptr<Layer> StreamChunk::layer() const { return d->layer; }

void StreamChunk::setLayer(const std::shared_ptr<Layer> &layer) {
  d->layer = layer;
}

void StreamChunk::setPayloadBuffer(v8::Local<v8::Object> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  if (Buffer *buffer = v8pp::class_<Buffer>::unwrap_object(isolate, obj)) {
    d->payload = buffer->slice();
  }
}

v8::Local<v8::Object> StreamChunk::payloadBuffer() const {
  Isolate *isolate = Isolate::GetCurrent();
  return v8pp::class_<Buffer>::import_external(isolate,
                                               d->payload->slice().release());
}

void StreamChunk::setEnd(bool end) { d->end = end; }

bool StreamChunk::end() const { return d->end; }

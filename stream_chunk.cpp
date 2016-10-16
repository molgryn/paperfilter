#include "stream_chunk.hpp"
#include "buffer.hpp"
#include <v8pp/class.hpp>

using namespace v8;

class StreamChunk::Private {
public:
  std::string ns;
  std::string id;
  std::weak_ptr<Layer> layer;
  std::unique_ptr<Buffer> payload = std::unique_ptr<Buffer>(new Buffer());
  bool end = false;
};

StreamChunk::StreamChunk(const std::string &ns, const std::string &id)
    : d(std::make_shared<Private>()) {
  d->ns = ns;
  d->id = id;
}

StreamChunk::StreamChunk(const StreamChunk &stream,
                         const std::shared_ptr<Layer> &layer)
    : d(stream.d) {
  d->layer = layer;
}

StreamChunk::~StreamChunk() {}

std::string StreamChunk::ns() const { return d->ns; }

std::string StreamChunk::id() const { return d->id; }

std::shared_ptr<Layer> StreamChunk::layer() const { return d->layer.lock(); }

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

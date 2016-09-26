#include "stream_chunk.hpp"

class StreamChunk::Private {
public:
  std::string ns;
  std::string id;
  std::weak_ptr<Layer> layer;
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

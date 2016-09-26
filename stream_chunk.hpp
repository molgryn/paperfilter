#ifndef STREAM_CHUNK_HPP
#define STREAM_CHUNK_HPP

#include <memory>
#include <string>

class Layer;

class StreamChunk {
public:
  StreamChunk(const std::string &ns, const std::string &id);
  StreamChunk(const StreamChunk &stream, const std::shared_ptr<Layer> &layer);
  ~StreamChunk();
  StreamChunk(const StreamChunk &) = delete;
  StreamChunk &operator=(const StreamChunk &) = delete;
  std::string ns() const;
  std::string id() const;
  std::shared_ptr<Layer> layer() const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

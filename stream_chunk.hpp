#ifndef STREAM_CHUNK_HPP
#define STREAM_CHUNK_HPP

#include <memory>
#include <string>
#include <v8.h>

class Layer;

class StreamChunk {
public:
  StreamChunk(const std::string &ns, const std::string &id,
              v8::Local<v8::Object> obj);
  StreamChunk(const StreamChunk &stream);
  ~StreamChunk();
  StreamChunk &operator=(const StreamChunk &) = delete;
  std::string ns() const;
  std::string id() const;
  std::shared_ptr<Layer> layer() const;
  void setLayer(const std::shared_ptr<Layer> &layer);
  void setPayloadBuffer(v8::Local<v8::Object>);
  v8::Local<v8::Object> payloadBuffer() const;
  void setEnd(bool end);
  bool end() const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

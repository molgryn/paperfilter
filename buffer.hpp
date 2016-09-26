#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <memory>
#include <v8.h>
#include <vector>

class Buffer {
public:
  Buffer(const std::shared_ptr<const std::vector<char>> &source);
  explicit Buffer(const v8::FunctionCallbackInfo<v8::Value> &args);
  ~Buffer();
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;
  size_t length() const;
  std::unique_ptr<Buffer> slice(size_t start, size_t end) const;
  std::unique_ptr<Buffer> slice(size_t start = 0) const;

  void readInt8(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void readInt16BE(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void readInt32BE(const v8::FunctionCallbackInfo<v8::Value> &args) const;

  void readUInt8(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void readUInt16BE(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void readUInt32BE(const v8::FunctionCallbackInfo<v8::Value> &args) const;

  void sliceBuffer(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void toString(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  void indexOf(const v8::FunctionCallbackInfo<v8::Value> &args) const;
  std::string valueOf() const;
  const char *data(size_t offset = 0) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

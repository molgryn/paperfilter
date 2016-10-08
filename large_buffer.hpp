#ifndef LARGE_BUFFER_HPP
#define LARGE_BUFFER_HPP

#include <memory>
#include <string>
#include <v8.h>

class LargeBuffer {
public:
  LargeBuffer(const std::string &id = "");
  ~LargeBuffer();
  LargeBuffer(const LargeBuffer &) = delete;
  LargeBuffer &operator=(const LargeBuffer &) = delete;

  std::string id() const;
  void write(const v8::FunctionCallbackInfo<v8::Value> &args);
  void read(const v8::FunctionCallbackInfo<v8::Value> &args);
  static std::string tmpDir();

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

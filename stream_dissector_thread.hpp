#ifndef STREAM_DISSECTOR_THREAD_HPP
#define STREAM_DISSECTOR_THREAD_HPP

#include "dissector.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class StreamChunk;

class StreamDissectorThread {
public:
  struct Context {
    std::string tmpDir;
    std::vector<Dissector> dissectors;
    std::function<void(std::string)> errorCb;
    std::function<void(std::vector<std::unique_ptr<StreamChunk>>)> streamsCb;
  };

public:
  StreamDissectorThread(const std::shared_ptr<Context> &ctx);
  ~StreamDissectorThread();
  StreamDissectorThread(const StreamDissectorThread &) = delete;
  StreamDissectorThread &operator=(const StreamDissectorThread &) = delete;
  void insert(std::unique_ptr<StreamChunk> chunk);
  void clearStream(const std::string &ns, const std::string &id);

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

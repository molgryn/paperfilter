#ifndef STREAM_DISPATCHER_HPP
#define STREAM_DISPATCHER_HPP

#include "dissector.hpp"
#include <functional>
#include <memory>
#include <vector>

class PacketStore;
class StreamChunk;

class StreamDispatcher {
public:
  struct Context {
    std::string tmpDir;
    std::vector<Dissector> dissectors;
    std::function<void(std::string)> errorCb;
    std::function<void(std::vector<std::unique_ptr<StreamChunk>>)> streamsCb;
  };

public:
  StreamDispatcher(const std::shared_ptr<Context> &ctx);
  ~StreamDispatcher();
  StreamDispatcher(const StreamDispatcher &) = delete;
  StreamDispatcher &operator=(const StreamDispatcher &) = delete;
  void insert(uint32_t seq,
              std::vector<std::unique_ptr<StreamChunk>> streamChunks);
  void insert(std::vector<std::unique_ptr<StreamChunk>> streamChunks);

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

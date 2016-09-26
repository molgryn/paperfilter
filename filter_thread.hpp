#ifndef FILTER_THREAD_HPP
#define FILTER_THREAD_HPP

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <string>
#include <condition_variable>

class Packet;
class PacketStore;

class FilterThread {
public:
  struct Context {
    std::mutex mutex;
    std::condition_variable cond;
    std::vector<uint32_t> filterdPackets;
    uint32_t maxSeq = 0;
    PacketStore *store = nullptr;
    std::string filter;
    std::function<void(uint32_t)> updateCb;
    std::function<void(std::string)> errorCb;
  };

public:
  FilterThread(const std::shared_ptr<Context> &ctx);
  ~FilterThread();
  FilterThread(const FilterThread &) = delete;
  FilterThread &operator=(const FilterThread &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

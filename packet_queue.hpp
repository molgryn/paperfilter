#ifndef PACKET_QUEUE_HPP
#define PACKET_QUEUE_HPP

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

class Packet;

class PacketQueue {
public:
  void push(const std::shared_ptr<Packet> &packet);
  std::shared_ptr<Packet> pop();
  void close();

private:
  std::queue<std::shared_ptr<Packet>> queue;
  std::mutex mutex;
  std::condition_variable cond;
  bool closed = false;
};

#endif

#include "packet_queue.hpp"
#include "packet.hpp"

void PacketQueue::push(const std::shared_ptr<Packet> &pkt) {
  std::lock_guard<std::mutex> lock(mutex);
  if (closed)
    return;
  queue.push(std::move(pkt));
  cond.notify_one();
}

std::shared_ptr<Packet> PacketQueue::pop() {
  std::unique_lock<std::mutex> lock(mutex);
  cond.wait(lock, [this] { return !queue.empty() || closed; });
  if (closed)
    return std::shared_ptr<Packet>();
  std::shared_ptr<Packet> pkt = std::move(queue.front());
  queue.pop();
  return pkt;
}

void PacketQueue::close() {
  std::lock_guard<std::mutex> lock(mutex);
  closed = true;
  cond.notify_all();
}

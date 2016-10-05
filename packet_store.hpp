#ifndef PACKET_STORE_HPP
#define PACKET_STORE_HPP

#include <uv.h>
#include <memory>
#include <vector>
#include <functional>

class Packet;

class PacketStore {
public:
  PacketStore();
  ~PacketStore();
  void insert(const std::shared_ptr<Packet> &pkt);
  std::vector<std::shared_ptr<Packet>> get(uint32_t start, uint32_t end) const;
  std::shared_ptr<Packet> get(uint32_t seq) const;
  uint32_t maxSeq() const;
  int addHandler(const std::function<void(uint32_t)> &cb);
  void removeHandler(int id);

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

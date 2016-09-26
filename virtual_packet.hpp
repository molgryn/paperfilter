#ifndef VIRTUAL_PACKET_HPP
#define VIRTUAL_PACKET_HPP

#include <memory>

class VirtualPacket {
public:
  VirtualPacket();
  ~VirtualPacket();
  VirtualPacket(const VirtualPacket &) = delete;
  VirtualPacket &operator=(const VirtualPacket &) = delete;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

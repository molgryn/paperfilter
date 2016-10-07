#ifndef VIRTUAL_PACKET_HPP
#define VIRTUAL_PACKET_HPP

#include <memory>
#include <string>
#include <v8.h>

class VirtualPacket {
public:
  VirtualPacket(const std::string &ns);
  ~VirtualPacket();
  VirtualPacket &operator=(const VirtualPacket &) = delete;
  std::string ns() const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

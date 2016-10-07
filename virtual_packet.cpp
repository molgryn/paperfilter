#include "virtual_packet.hpp"
#include "buffer.hpp"
#include <v8pp/class.hpp>

class VirtualPacket::Private {
public:
  std::string ns;
};

VirtualPacket::VirtualPacket(const std::string &ns)
    : d(std::make_shared<Private>()) {
  d->ns = ns;
}

VirtualPacket::~VirtualPacket() {}

std::string VirtualPacket::ns() const { return d->ns; }

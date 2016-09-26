#include "virtual_packet.hpp"

class VirtualPacket::Private {};

VirtualPacket::VirtualPacket() : d(std::make_shared<Private>()) {}

VirtualPacket::~VirtualPacket() {}

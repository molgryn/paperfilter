#include "virtual_packet.hpp"
#include "buffer.hpp"
#include <v8pp/class.hpp>

class VirtualPacket::Private {
public:
  std::string ns;
  std::shared_ptr<std::vector<char>> payload = std::make_shared<std::vector<char>>();
};

VirtualPacket::VirtualPacket(const std::string &ns)
    : d(std::make_shared<Private>()) {
  d->ns = ns;
}

VirtualPacket::~VirtualPacket() {}

std::string VirtualPacket::ns() const { return d->ns; }

std::shared_ptr<const std::vector<char>> VirtualPacket::payload() const {
  return d->payload;
}

v8::Local<v8::Object> VirtualPacket::payloadBuffer() const {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  return v8pp::class_<Buffer>::create_object(isolate, d->payload);
}

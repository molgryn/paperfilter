#include "virtual_packet.hpp"
#include "buffer.hpp"
#include <v8pp/class.hpp>

using namespace v8;

class VirtualPacket::Private {
public:
  std::string ns;
  std::unique_ptr<Buffer> payload = std::unique_ptr<Buffer>(new Buffer());
};

VirtualPacket::VirtualPacket(const std::string &ns)
    : d(std::make_shared<Private>()) {
  d->ns = ns;
}

VirtualPacket::~VirtualPacket() {}

std::string VirtualPacket::ns() const { return d->ns; }

std::unique_ptr<Buffer> VirtualPacket::payload() const {
  return d->payload->slice();
}

v8::Local<v8::Object> VirtualPacket::payloadBuffer() const {
  Isolate *isolate = Isolate::GetCurrent();
  return v8pp::class_<Buffer>::import_external(isolate,
                                               d->payload->slice().release());
}

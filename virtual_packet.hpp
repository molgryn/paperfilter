#ifndef VIRTUAL_PACKET_HPP
#define VIRTUAL_PACKET_HPP

#include <memory>
#include <string>
#include <v8.h>

class Buffer;

class VirtualPacket {
public:
  VirtualPacket(const std::string &ns);
  ~VirtualPacket();
  VirtualPacket &operator=(const VirtualPacket &) = delete;
  std::string ns() const;

  std::unique_ptr<Buffer> payload() const;
  void setPayloadBuffer(v8::Local<v8::Object>);
  v8::Local<v8::Object> payloadBuffer() const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

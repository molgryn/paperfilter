#ifndef PACKET_HPP
#define PACKET_HPP

#include <v8.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class Layer;

class Packet {
public:
  Packet(v8::Local<v8::Object> option);
  ~Packet();
  Packet(const Packet &) = delete;
  Packet &operator=(const Packet &) = delete;

  uint32_t seq() const;
  void setSeq(uint32_t id);

  uint32_t ts_sec() const;
  uint32_t ts_nsec() const;
  uint32_t length() const;
  std::string summary() const;
  std::string extension() const;

  std::shared_ptr<const std::vector<char>> payload() const;
  v8::Local<v8::Object> payloadBuffer() const;

  void addLayer(const std::shared_ptr<Layer> &layer);
  const std::unordered_map<std::string, std::shared_ptr<Layer>> &layers() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

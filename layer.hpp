#ifndef LAYER_HPP
#define LAYER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <v8.h>
#include <vector>

class Packet;
class Item;
class ItemValue;
class Buffer;

class Layer {
public:
  Layer(const std::string &ns);
  ~Layer();
  Layer &operator=(const Layer &) = delete;

  std::string ns() const;
  void setNs(const std::string &ns);
  std::string name() const;
  void setName(const std::string &name);
  std::string alias() const;
  void setAlias(const std::string &name);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::string extension() const;
  void setExtension(const std::string &extension);

  void addLayer(const std::shared_ptr<Layer> &layer);
  std::unordered_map<std::string, std::shared_ptr<Layer>> &layers() const;
  v8::Local<v8::Object> layersObject() const;

  void setPacket(const std::shared_ptr<Packet> &pkt);
  std::shared_ptr<Packet> packet() const;

  void addItem(v8::Local<v8::Object> obj);
  std::vector<Item> items() const;

  std::unique_ptr<Buffer> payload() const;
  void setPayload(std::unique_ptr<Buffer> buffer);
  void setPayloadBuffer(v8::Local<v8::Object> obj);
  v8::Local<v8::Object> payloadBuffer() const;

  void setAttr(const std::string &name, v8::Local<v8::Object> obj);
  std::unordered_map<std::string, ItemValue> attrs() const;
  v8::Local<v8::Value> attr(const std::string &name) const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

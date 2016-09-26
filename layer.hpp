#ifndef LAYER_HPP
#define LAYER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Packet;

class Layer {
public:
  Layer(const std::string &ns);
  ~Layer();
  Layer &operator=(const Layer &) = delete;

  std::string ns() const;
  std::string name() const;
  void setName(const std::string &name);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::string extension() const;
  void setExtension(const std::string &extension);

  void addLayer(const std::shared_ptr<Layer> &layer);
  std::unordered_map<std::string, std::shared_ptr<Layer>> &layers() const;

  void setPacket(const std::shared_ptr<Packet> &pkt);
  std::shared_ptr<Packet> packet() const;

private:
  class Private;
  std::shared_ptr<Private> d;
};

#endif

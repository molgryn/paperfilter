#include "layer.hpp"
#include "item.hpp"
#include <v8pp/class.hpp>

using namespace v8;

class Layer::Private {
public:
  std::string ns;
  std::string name;
  std::string summary;
  std::string extension;
  std::unordered_map<std::string, std::shared_ptr<Layer>> layers;
  std::weak_ptr<Packet> pkt;
  std::vector<Item> items;
};

Layer::Layer(const std::string &ns) : d(std::make_shared<Private>()) {
  d->ns = ns;
}

Layer::~Layer() {}

std::string Layer::ns() const { return d->ns; }

std::string Layer::name() const { return d->name; }

void Layer::setName(const std::string &name) { d->name = name; }

std::string Layer::summary() const { return d->summary; };

void Layer::setSummary(const std::string &summary) { d->summary = summary; }

std::string Layer::extension() const { return d->extension; };

void Layer::setExtension(const std::string &extension) {
  d->extension = extension;
}

void Layer::addLayer(const std::shared_ptr<Layer> &layer) {
  d->layers[layer->ns()] = std::move(layer);
}

std::unordered_map<std::string, std::shared_ptr<Layer>> &Layer::layers() const {
  return d->layers;
}

void Layer::setPacket(const std::shared_ptr<Packet> &pkt) { d->pkt = pkt; }

std::shared_ptr<Packet> Layer::packet() const { return d->pkt.lock(); }

void Layer::addItem(v8::Local<v8::Object> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  if (Item *item = v8pp::class_<Item>::unwrap_object(isolate, obj)) {
    d->items.emplace_back(*item);
  }
}

std::vector<Item> Layer::items() const { return d->items; }

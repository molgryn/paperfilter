#include "item.hpp"
#include "item_value.hpp"
#include <v8pp/class.hpp>
#include <vector>

using namespace v8;

class Item::Private {
public:
  std::string name;
  std::string attr;
  std::string range;
  ItemValue value;
  std::vector<Item> children;
};

Item::Item() : d(new Private()) {}

Item::Item(const Item &item) : d(new Private(*item.d)) {}

Item::~Item() {}

std::string Item::name() const { return d->name; }

void Item::setName(const std::string &name) { d->name = name; }

std::string Item::attr() const { return d->attr; }

void Item::setAttr(const std::string &attr) { d->attr = attr; }

std::string Item::range() const { return d->range; }

void Item::setRange(const std::string &range) { d->range = range; }

v8::Local<v8::Object> Item::valueObject() const {
  Isolate *isolate = Isolate::GetCurrent();
  return v8pp::class_<ItemValue>::import_external(isolate,
                                                  new ItemValue(d->value));
}

ItemValue Item::value() const { return d->value; }

void Item::setValue(v8::Local<v8::Object> value) {
  Isolate *isolate = Isolate::GetCurrent();
  if (ItemValue *iv = v8pp::class_<ItemValue>::unwrap_object(isolate, value)) {
    d->value = *iv;
  }
}

std::vector<Item> Item::children() const { return d->children; }

void Item::addChild(v8::Local<v8::Object> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  if (Item *item = v8pp::class_<Item>::unwrap_object(isolate, obj)) {
    d->children.emplace_back(*item);
  }
}

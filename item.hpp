#ifndef ITEM_HPP
#define ITEM_HPP

#include "item_value.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <v8.h>
#include <vector>

class Item {
public:
  Item();
  Item(const v8::FunctionCallbackInfo<v8::Value> &args);
  Item(v8::Local<v8::Value> value);
  Item(const Item &item);
  ~Item();

  std::string name() const;
  void setName(const std::string &name);
  std::string range() const;
  void setRange(const std::string &range);
  v8::Local<v8::Object> valueObject() const;
  ItemValue value() const;
  void setValue(v8::Local<v8::Object> value);

  std::vector<Item> children() const;
  void addChild(v8::Local<v8::Object> obj);

  void setAttrObject(const std::string &name, v8::Local<v8::Object> obj);
  std::unordered_map<std::string, ItemValue> attrs() const;
  v8::Local<v8::Value> attr(const std::string &name) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

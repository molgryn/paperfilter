#ifndef ITEM_HPP
#define ITEM_HPP

#include "item_value.hpp"
#include <memory>
#include <string>
#include <v8.h>
#include <vector>

class Item {
public:
  Item();
  Item(const v8::FunctionCallbackInfo<v8::Value> &args);
  Item(const Item &item);
  ~Item();

  std::string name() const;
  void setName(const std::string &name);
  std::string attr() const;
  void setAttr(const std::string &attr);
  std::string range() const;
  void setRange(const std::string &range);
  v8::Local<v8::Object> valueObject() const;
  ItemValue value() const;
  void setValue(v8::Local<v8::Object> value);

  std::vector<Item> children() const;
  void addChild(v8::Local<v8::Object> obj);

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

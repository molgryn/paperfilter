#ifndef ITEM_HPP
#define ITEM_HPP

#include <memory>
#include <string>
#include <v8.h>
#include <vector>

class Item {
public:
  Item();
  Item(const Item &item);
  ~Item();

  std::string name() const;
  void setName(const std::string &name);
  std::string attr() const;
  void setAttr(const std::string &attr);
  std::string range() const;
  void setRange(const std::string &range);
  v8::Local<v8::Object> value() const;
  void setValue(v8::Local<v8::Object> value);

  const std::vector<std::unique_ptr<Item>> &children() const;
  void addChild(v8::Local<v8::Object> obj);

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

#ifndef ITEM_HPP
#define ITEM_HPP

#include <memory>
#include <string>
#include <v8.h>

class Item {
public:
  Item();
  ~Item();

  std::string name() const;
  void setName(const std::string &name);
  std::string attr() const;
  void setAttr(const std::string &attr);
  std::string range() const;
  void setRange(const std::string &range);
  v8::Local<v8::Object> value() const;
  void setValue(v8::Local<v8::Object> value);

  v8::Local<v8::Array> children() const;
  void addChild(v8::Local<v8::Object> item);

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

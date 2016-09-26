#ifndef ITEM_VALUE_HPP
#define ITEM_VALUE_HPP

#include <memory>
#include <string>
#include <v8.h>

class Buffer;

class ItemValue {
public:
  enum BaseType { NUL, NUMBER, BOOLEAN, STRING, BUFFER, JSON };

public:
  explicit ItemValue(const v8::FunctionCallbackInfo<v8::Value> &args);
  ~ItemValue();
  v8::Local<v8::Value> value() const;
  std::string type() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

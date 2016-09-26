#include "item_value.hpp"
#include "buffer.hpp"
#include <memory>
#include <nan.h>
#include <node_buffer.h>
#include <v8pp/class.hpp>
#include <v8pp/json.hpp>

class ItemValue::Private {
public:
  Private();
  ~Private();

public:
  BaseType base = NUL;
  double num;
  std::string str;
  std::shared_ptr<Buffer> buf;
  std::string type;
};

ItemValue::Private::Private() {}

ItemValue::Private::~Private() {}

ItemValue::ItemValue(const v8::FunctionCallbackInfo<v8::Value> &args)
    : d(new Private()) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Value> val = args[0];
  if (!val.IsEmpty()) {
    if (val->IsNumber()) {
      d->num = val->NumberValue();
      d->base = NUMBER;
    } else if (val->IsBoolean()) {
      d->num = val->BooleanValue();
      d->base = BOOLEAN;
    } else if (val->IsString()) {
      d->str = v8pp::from_v8<std::string>(isolate, val, "");
      d->base = STRING;
    } else if (val->IsObject()) {
      if (Buffer *buffer = v8pp::class_<Buffer>::unwrap_object(isolate, val)) {
        d->buf = buffer->slice();
        d->base = BUFFER;
      } else {
        d->str = v8pp::json_str(isolate, val);
        d->base = JSON;
      }
    }
  }
}

ItemValue::~ItemValue() {}

v8::Local<v8::Value> ItemValue::value() const {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Value> val = v8::Null(isolate);
  switch (d->base) {
  case NUMBER:
    val = v8pp::to_v8(isolate, d->num);
    break;
  case BOOLEAN:
    val = v8pp::to_v8(isolate, static_cast<bool>(d->num));
    break;
  case STRING:
    val = v8pp::to_v8(isolate, d->str);
    break;
  case BUFFER:
    if (d->buf) {
      if (isolate->GetData(1)) { // node.js
        val = node::Buffer::New(
                  isolate, const_cast<char *>(d->buf->data()), d->buf->length(),
                  [](char *data, void *hint) {
                    delete static_cast<std::shared_ptr<const Buffer> *>(hint);
                  },
                  new std::shared_ptr<const Buffer>(d->buf))
                  .ToLocalChecked();
      } else { // dissector
        val = v8pp::class_<Buffer>::import_external(isolate,
                                                    d->buf->slice().release());
      }
    }
    break;
  case JSON:
    val = v8pp::json_parse(isolate, d->str);
    break;
  default:;
  }
  return val;
}

std::string ItemValue::type() const { return d->type; }

#ifndef SESSION_LAYER_WRAPPER_HPP
#define SESSION_LAYER_WRAPPER_HPP

#include "layer.hpp"
#include "buffer.hpp"
#include "session_item_wrapper.hpp"
#include <nan.h>
#include <node_buffer.h>
#include <v8pp/class.hpp>
#include <v8pp/json.hpp>

class SessionLayerWrapper : public Nan::ObjectWrap {
private:
  SessionLayerWrapper(const std::weak_ptr<const Layer> &layer) : layer(layer) {}
  SessionLayerWrapper(const SessionLayerWrapper &) = delete;
  SessionLayerWrapper &operator=(const SessionLayerWrapper &) = delete;

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
    v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
    Nan::SetAccessor(otl, Nan::New("namespace").ToLocalChecked(), ns);
    Nan::SetAccessor(otl, Nan::New("name").ToLocalChecked(), name);
    Nan::SetAccessor(otl, Nan::New("summary").ToLocalChecked(), summary);
    Nan::SetAccessor(otl, Nan::New("layers").ToLocalChecked(), layers);
    Nan::SetAccessor(otl, Nan::New("payload").ToLocalChecked(), payload);
    Nan::SetAccessor(otl, Nan::New("items").ToLocalChecked(), items);
    Nan::SetAccessor(otl, Nan::New("attrs").ToLocalChecked(), attrs);
    Nan::SetAccessor(otl, Nan::New("extension").ToLocalChecked(), extension);
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
  }

  static NAN_METHOD(New) { info.GetReturnValue().Set(info.This()); }

  static inline Nan::Persistent<v8::Function> &constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

  static NAN_GETTER(ns) {
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());
    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock())
      info.GetReturnValue().Set(
          v8pp::to_v8(v8::Isolate::GetCurrent(), layer->ns()));
  }

  static NAN_GETTER(name) {
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());
    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock())
      info.GetReturnValue().Set(
          v8pp::to_v8(v8::Isolate::GetCurrent(), layer->name()));
  }

  static NAN_GETTER(summary) {
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());
    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock())
      info.GetReturnValue().Set(
          v8pp::to_v8(v8::Isolate::GetCurrent(), layer->summary()));
  }

  static NAN_GETTER(extension) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());
    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock()) {
      v8::Local<v8::Value> ext = v8pp::json_parse(isolate, layer->extension());
      if (ext.IsEmpty())
        ext = v8::Object::New(isolate);
      info.GetReturnValue().Set(ext);
    }
  }

  static NAN_GETTER(layers) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());

    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock()) {
      v8::Local<v8::Object> obj = v8::Object::New(isolate);
      for (const auto &pair : layer->layers()) {
        obj->Set(v8pp::to_v8(isolate, pair.first),
                 SessionLayerWrapper::create(pair.second));
      }

      info.GetReturnValue().Set(obj);
    }
  }

  static NAN_GETTER(items) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());

    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock()) {
      const auto &items = layer->items();
      v8::Local<v8::Array> array = v8::Array::New(isolate, items.size());
      for (size_t i = 0; i < items.size(); ++i) {
        array->Set(i, SessionItemWrapper::create(items[i]));
      }
      info.GetReturnValue().Set(array);
    }
  }

  static NAN_GETTER(attrs) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());

    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock()) {
      const auto &attrs = layer->attrs();
      v8::Local<v8::Object> obj = v8::Object::New(isolate);
      for (const auto &pair : attrs) {
        obj->Set(v8pp::to_v8(isolate, pair.first),
                 SessionItemValueWrapper::create(pair.second));
      }
      info.GetReturnValue().Set(obj);
    }
  }

  static NAN_GETTER(payload) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionLayerWrapper *wrapper =
        ObjectWrap::Unwrap<SessionLayerWrapper>(info.Holder());

    if (const std::shared_ptr<const Layer> &layer = wrapper->layer.lock()) {
      Buffer *buf = layer->payload().release();
      v8::Local<v8::Object> buffer =
          node::Buffer::New(isolate, const_cast<char *>(buf->data()),
                            buf->length(),
                            [](char *data, void *hint) {
                              delete static_cast<Buffer *>(hint);
                            },
                            buf)
              .ToLocalChecked();
      info.GetReturnValue().Set(buffer);
    }
  }

  static v8::Local<v8::Object> create(const std::weak_ptr<const Layer> &layer) {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    v8::Local<v8::Value> argv[1] = {
        v8::Isolate::GetCurrent()->GetCurrentContext()->Global()};
    v8::Local<v8::Object> obj = cons->NewInstance(1, argv);
    SessionLayerWrapper *wrapper = new SessionLayerWrapper(layer);
    wrapper->Wrap(obj);
    return obj;
  }

private:
  std::weak_ptr<const Layer> layer;
};

#endif

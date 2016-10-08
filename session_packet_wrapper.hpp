#ifndef PACKET_WRAPPER_HPP
#define PACKET_WRAPPER_HPP

#include "buffer.hpp"
#include "packet.hpp"
#include "session_layer_wrapper.hpp"
#include <nan.h>
#include <node_buffer.h>

class SessionPacketWrapper : public Nan::ObjectWrap {
private:
  SessionPacketWrapper(const std::weak_ptr<const Packet> &pkt) : pkt(pkt) {}
  SessionPacketWrapper(const SessionPacketWrapper &) = delete;
  SessionPacketWrapper &operator=(const SessionPacketWrapper &) = delete;

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("Packet").ToLocalChecked());
    v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
    Nan::SetAccessor(otl, Nan::New("seq").ToLocalChecked(), seq);
    Nan::SetAccessor(otl, Nan::New("ts_sec").ToLocalChecked(), ts_sec);
    Nan::SetAccessor(otl, Nan::New("ts_nsec").ToLocalChecked(), ts_nsec);
    Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);
    Nan::SetAccessor(otl, Nan::New("summary").ToLocalChecked(), summary);
    Nan::SetAccessor(otl, Nan::New("extension").ToLocalChecked(), extension);
    Nan::SetAccessor(otl, Nan::New("payload").ToLocalChecked(), payload);
    Nan::SetAccessor(otl, Nan::New("layers").ToLocalChecked(), layers);
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
  }

  static NAN_METHOD(New) { info.GetReturnValue().Set(info.This()); }

  static NAN_GETTER(seq) {
    SessionPacketWrapper *obj =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = obj->pkt.lock())
      info.GetReturnValue().Set(pkt->seq());
  }

  static NAN_GETTER(ts_sec) {
    SessionPacketWrapper *obj =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = obj->pkt.lock())
      info.GetReturnValue().Set(pkt->ts_sec());
  }

  static NAN_GETTER(ts_nsec) {
    SessionPacketWrapper *obj =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = obj->pkt.lock())
      info.GetReturnValue().Set(pkt->ts_nsec());
  }

  static NAN_GETTER(length) {
    SessionPacketWrapper *obj =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = obj->pkt.lock())
      info.GetReturnValue().Set(pkt->length());
  }

  static NAN_GETTER(payload) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionPacketWrapper *wrapper =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());

    if (const std::shared_ptr<const Packet> &pkt = wrapper->pkt.lock()) {
      Buffer *buf = pkt->payload().release();
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

  static NAN_GETTER(summary) {
    SessionPacketWrapper *wrapper =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = wrapper->pkt.lock())
      info.GetReturnValue().Set(
          v8pp::to_v8(v8::Isolate::GetCurrent(), pkt->summary()));
  }

  static NAN_GETTER(extension) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionPacketWrapper *wrapper =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());
    if (const std::shared_ptr<const Packet> &pkt = wrapper->pkt.lock()) {
      v8::Local<v8::Value> ext = v8pp::json_parse(isolate, pkt->extension());
      if (ext.IsEmpty())
        ext = v8::Object::New(isolate);
      info.GetReturnValue().Set(ext);
    }
  }

  static NAN_GETTER(layers) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SessionPacketWrapper *wrapper =
        ObjectWrap::Unwrap<SessionPacketWrapper>(info.Holder());

    if (const std::shared_ptr<const Packet> &pkt = wrapper->pkt.lock()) {
      v8::Local<v8::Object> obj = v8::Object::New(isolate);
      for (const auto &pair : pkt->layers()) {
        obj->Set(v8pp::to_v8(isolate, pair.first),
                 SessionLayerWrapper::create(pair.second));
      }

      info.GetReturnValue().Set(obj);
    }
  }

  static inline Nan::Persistent<v8::Function> &constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

  static v8::Local<v8::Object> create(const std::weak_ptr<const Packet> &pkt) {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    v8::Local<v8::Value> argv[1] = {
        v8::Isolate::GetCurrent()->GetCurrentContext()->Global()};
    v8::Local<v8::Object> obj = cons->NewInstance(1, argv);
    SessionPacketWrapper *wrapper = new SessionPacketWrapper(pkt);
    wrapper->Wrap(obj);
    return obj;
  }

private:
  std::weak_ptr<const Packet> pkt;
};

#endif

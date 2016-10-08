#ifndef SESSION_WRAPPER_HPP
#define SESSION_WRAPPER_HPP

#include "session.hpp"
#include "session_packet_wrapper.hpp"
#include <nan.h>
#include <node_buffer.h>

class SessionWrapper : public Nan::ObjectWrap {
private:
  SessionWrapper(Session *session) : session(session) {}
  SessionWrapper(const SessionWrapper &) = delete;
  SessionWrapper &operator=(const SessionWrapper &) = delete;

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("Session").ToLocalChecked());
    SetPrototypeMethod(tpl, "analyze", analyze);
    SetPrototypeMethod(tpl, "filter", filter);
    SetPrototypeMethod(tpl, "get", get);
    v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
    Nan::SetAccessor(otl, Nan::New("errorCallback").ToLocalChecked(),
                     errorCallback, setErrorCallback);
    Nan::SetAccessor(otl, Nan::New("packetCallback").ToLocalChecked(),
                     packetCallback, setPacketCallback);
    Nan::SetAccessor(otl, Nan::New("filterCallback").ToLocalChecked(),
                     filterCallback, setFilterCallback);
    Nan::SetAccessor(otl, Nan::New("namespace").ToLocalChecked(), ns);
    Nan::SetAccessor(otl, Nan::New("permission").ToLocalChecked(), permission);
    Nan::SetAccessor(otl, Nan::New("devices").ToLocalChecked(), devices);
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("Session").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
  }

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      SessionWrapper *obj = new SessionWrapper(new Session(info[0]));
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }

  static NAN_METHOD(analyze) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    auto obj = Nan::To<v8::Object>(info[0]);
    if (!obj.IsEmpty()) {
      std::unique_ptr<Packet> pkt(new Packet(obj.ToLocalChecked()));
      wrapper->session->analyze(std::move(pkt));
    }
  }

  static NAN_METHOD(filter) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    const auto &name = Nan::Utf8String(info[0]);
    const auto &filter = Nan::Utf8String(info[1]);
    if (*name && *filter) {
      wrapper->session->filter(*name, *filter);
    }
  }

  static NAN_GETTER(errorCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(wrapper->session->errorCallback());
  }

  static NAN_SETTER(setErrorCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    if (!value.IsEmpty() && value->IsFunction()) {
      wrapper->session->setErrorCallback(value.As<v8::Function>());
    }
  }

  static NAN_GETTER(packetCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(wrapper->session->packetCallback());
  }

  static NAN_SETTER(setPacketCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    if (!value.IsEmpty() && value->IsFunction()) {
      wrapper->session->setPacketCallback(value.As<v8::Function>());
    }
  }

  static NAN_GETTER(filterCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(wrapper->session->filterCallback());
  }

  static NAN_SETTER(setFilterCallback) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    if (!value.IsEmpty() && value->IsFunction()) {
      wrapper->session->setFilterCallback(value.As<v8::Function>());
    }
  }

  static NAN_METHOD(get) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    auto seq = Nan::To<uint32_t>(info[0]);
    if (seq.IsJust()) {
      auto obj =
          SessionPacketWrapper::create(wrapper->session->get(seq.FromJust()));
      info.GetReturnValue().Set(obj);
    }
  }

  static NAN_GETTER(ns) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(
        Nan::New(wrapper->session->ns()).ToLocalChecked());
  }

  static NAN_GETTER(permission) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(
        Nan::New(wrapper->session->permission()));
  }

  static NAN_GETTER(devices) {
    SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
    info.GetReturnValue().Set(wrapper->session->devices());
  }

  static inline Nan::Persistent<v8::Function> &constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

private:
  std::unique_ptr<Session> session;
};

#endif

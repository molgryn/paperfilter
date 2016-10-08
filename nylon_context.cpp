#include "nylon_context.hpp"
#include "buffer.hpp"
#include "item_value.hpp"
#include "layer.hpp"
#include "item.hpp"
#include "packet.hpp"
#include "stream_chunk.hpp"
#include "virtual_packet.hpp"
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>

using namespace v8;

void NylonContext::init(v8::Isolate *isolate) {
  v8pp::class_<Packet> Packet_class(isolate);
  Packet_class.set("seq", v8pp::property(&Packet::seq));
  Packet_class.set("ts_sec", v8pp::property(&Packet::ts_sec));
  Packet_class.set("ts_nsec", v8pp::property(&Packet::ts_nsec));
  Packet_class.set("length", v8pp::property(&Packet::length));
  Packet_class.set("payload", v8pp::property(&Packet::payloadBuffer));

  v8pp::class_<VirtualPacket> VirtualPacket_class(isolate);
  VirtualPacket_class.ctor<const std::string &>();
  VirtualPacket_class.set("payload",
                          v8pp::property(&VirtualPacket::payloadBuffer,
                                         &VirtualPacket::setPayloadBuffer));

  v8pp::class_<Buffer> Buffer_class(isolate);
  Buffer_class.ctor<const v8::FunctionCallbackInfo<v8::Value> &>();
  Buffer_class.set("from", &Buffer::from);
  Buffer_class.set("isBuffer", &Buffer::isBuffer);
  Buffer_class.set("length", v8pp::property(&Buffer::length));
  Buffer_class.set("slice", &Buffer::sliceBuffer);
  Buffer_class.set("toString", &Buffer::toString);
  Buffer_class.set("valueOf", &Buffer::valueOf);
  Buffer_class.set("indexOf", &Buffer::indexOf);
  Buffer_class.set("readInt8", &Buffer::readInt8);
  Buffer_class.set("readInt16BE", &Buffer::readInt16BE);
  Buffer_class.set("readInt32BE", &Buffer::readInt32BE);
  Buffer_class.set("readUInt8", &Buffer::readUInt8);
  Buffer_class.set("readUInt16BE", &Buffer::readUInt16BE);
  Buffer_class.set("readUInt32BE", &Buffer::readUInt32BE);

  v8pp::class_<Layer> Layer_class(isolate);
  Layer_class.ctor<const std::string &>();
  Layer_class.set("namespace", v8pp::property(&Layer::ns));
  Layer_class.set("name", v8pp::property(&Layer::name, &Layer::setName));
  Layer_class.set("summary",
                  v8pp::property(&Layer::summary, &Layer::setSummary));
  Layer_class.set("extension",
                  v8pp::property(&Layer::extension, &Layer::setExtension));
  Layer_class.set("addItem", &Layer::addItem);

  v8pp::class_<Item> Item_class(isolate);
  Item_class.ctor<>();
  Item_class.set("name", v8pp::property(&Item::name, &Item::setName));
  Item_class.set("attr", v8pp::property(&Item::attr, &Item::setAttr));
  Item_class.set("range", v8pp::property(&Item::range, &Item::setRange));
  Item_class.set("value", v8pp::property(&Item::value, &Item::setValue));
  Item_class.set("addChild", &Item::addChild);

  v8pp::class_<ItemValue> ItemValue_class(isolate);
  ItemValue_class.ctor<const v8::FunctionCallbackInfo<v8::Value> &>();

  v8pp::class_<StreamChunk> StreamChunk_class(isolate);
  StreamChunk_class.ctor<const std::string &, const std::string &>();
  StreamChunk_class.set("namespace", v8pp::property(&Layer::ns));

  v8pp::module dripcap(isolate);
  dripcap.set("Buffer", Buffer_class);
  dripcap.set("Layer", Layer_class);
  dripcap.set("Item", Item_class);
  dripcap.set("Value", ItemValue_class);
  dripcap.set("StreamChunk", StreamChunk_class);
  dripcap.set("VirtualPacket", VirtualPacket_class);

  Local<FunctionTemplate> require = FunctionTemplate::New(
      isolate, [](FunctionCallbackInfo<Value> const &args) {
        Isolate *isolate = Isolate::GetCurrent();
        const std::string &name =
            v8pp::from_v8<std::string>(isolate, args[0], "");
        if (name == "dripcap") {
          args.GetReturnValue().Set(args.Data());
        } else {
          std::string err("Cannot find module '");
          args.GetReturnValue().Set(
              v8pp::throw_ex(isolate, (err + name + "'").c_str()));
        }
      }, dripcap.new_instance());

  isolate->GetCurrentContext()->Global()->Set(
      v8::String::NewFromUtf8(isolate, "require"), require->GetFunction());
}

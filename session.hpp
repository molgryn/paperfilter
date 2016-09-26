#ifndef SESSION_HPP
#define SESSION_HPP

#include <v8.h>
#include <string>
#include <memory>

class Packet;

class Session {
public:
  Session(v8::Local<v8::Value> option);
  ~Session();
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  v8::Local<v8::Function> errorCallback() const;
  void setErrorCallback(const v8::Local<v8::Function> &cb);

  v8::Local<v8::Function> packetCallback() const;
  void setPacketCallback(const v8::Local<v8::Function> &cb);

  v8::Local<v8::Function> filterCallback() const;
  void setFilterCallback(const v8::Local<v8::Function> &cb);

  void analyze(std::unique_ptr<Packet> pkt);
  void filter(const std::string &name, const std::string &filter);
  std::shared_ptr<const Packet> get(uint32_t seq) const;

  std::string ns() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

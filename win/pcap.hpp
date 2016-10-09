#ifndef PCAP_HPP
#define PCAP_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifdef interface
#undef interface
#endif

class Packet;

class Pcap {
public:
  struct Context {
    std::function<void(std::unique_ptr<Packet>)> packetCb;
    std::function<void(std::string)> errorCb;
  };
  struct Device {
    std::string id;
    std::string name;
    std::string description;
    int link = 0;
    bool loopback = false;
  };

public:
  Pcap(const std::shared_ptr<Context> &ctx);
  ~Pcap();
  std::vector<Device> devices() const;
  void setInterface(const std::string &ifs);
  std::string interface() const;
  void setPromiscuous(bool promisc);
  bool promiscuous() const;
  void setSnaplen(int len);
  int snaplen() const;
  bool setBPF(const std::string &filter, std::string *error);

  void start();
  void stop();

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

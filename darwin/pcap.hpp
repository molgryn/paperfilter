#ifndef PCAP_HPP
#define PCAP_HPP

#include <memory>
#include <functional>
#include <string>
#include <vector>

class Pcap {
public:
  struct Context {
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

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

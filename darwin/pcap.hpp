#ifndef PCAP_HPP
#define PCAP_HPP

#include <memory>

class Pcap {
public:
  struct Context {};

public:
  Pcap(const std::shared_ptr<Context> &ctx);
  ~Pcap();

private:
  class Private;
  std::unique_ptr<Private> d;
};

#endif

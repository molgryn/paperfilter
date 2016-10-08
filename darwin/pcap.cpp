#include "pcap.hpp"
#include <signal.h>
#include <pcap.h>

class Pcap::Private {
public:
  Private(const std::shared_ptr<Context> &ctx);

public:
  std::shared_ptr<Context> ctx;
};

Pcap::Private::Private(const std::shared_ptr<Context> &ctx) : ctx(ctx) {}

Pcap::Pcap(const std::shared_ptr<Context> &ctx) : d(new Private(ctx)) {}

Pcap::~Pcap() {}

std::vector<Pcap::Device> Pcap::devices() const {
  std::vector<Device> devs;

  pcap_if_t *alldevsp;
  char err[PCAP_ERRBUF_SIZE] = {'\0'};
  if (pcap_findalldevs(&alldevsp, err) < 0) {
    if (d->ctx->errorCb) {
      d->ctx->errorCb(std::string("pcap_findalldevs() failed: ") + err);
    }
    return devs;
  }

  for (pcap_if_t *ifs = alldevsp; ifs; ifs = ifs->next) {
    Device dev;
    dev.id = ifs->name;
    dev.name = ifs->name;
    if (ifs->description)
      dev.description = ifs->description;
    dev.loopback = ifs->flags & PCAP_IF_LOOPBACK;
    dev.link = -1;

    pcap_t *pcap = pcap_open_live(ifs->name, 1600, false, 500, err);
    if (pcap) {
      dev.link = pcap_datalink(pcap);
      pcap_close(pcap);
    }

    devs.push_back(dev);
  }

  pcap_freealldevs(alldevsp);
  return devs;
}

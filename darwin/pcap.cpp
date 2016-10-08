#include "pcap.hpp"
#include <signal.h>

class Pcap::Private {
public:
  Private(const std::shared_ptr<Context> &ctx);

public:
  std::shared_ptr<Context> ctx;
};

Pcap::Private::Private(const std::shared_ptr<Context> &ctx) : ctx(ctx) {}

Pcap::Pcap(const std::shared_ptr<Context> &ctx) : d(new Private(ctx)) {}

Pcap::~Pcap() {}

#include "packet_store.hpp"
#include "packet.hpp"
#include <map>
#include <unordered_map>

class PacketStore::Private {
public:
  Private();
  ~Private();

public:
  uv_rwlock_t rwlock;
  std::unordered_map<int, std::function<void(uint32_t)>> handlers;
  uint32_t maxSeq = 0;
  std::map<uint32_t, std::shared_ptr<Packet>> packets;
};

PacketStore::Private::Private() { uv_rwlock_init(&rwlock); }

PacketStore::Private::~Private() { uv_rwlock_destroy(&rwlock); }

PacketStore::PacketStore() : d(new Private()) {}

PacketStore::~PacketStore() {}

void PacketStore::insert(const std::shared_ptr<Packet> &pkt) {
  uv_rwlock_wrlock(&d->rwlock);
  d->packets[pkt->seq()] = pkt;
  uint32_t seq = d->maxSeq;
  for (auto it = d->packets.find(seq + 1); it != d->packets.end();
       seq++, it = d->packets.find(seq + 1))
    ;
  if (d->maxSeq < seq) {
    d->maxSeq = seq;
    for (const auto &pair : d->handlers) {
      if (pair.second)
        pair.second(seq);
    }
  }
  uv_rwlock_wrunlock(&d->rwlock);
}

std::vector<std::shared_ptr<Packet>> PacketStore::get(uint32_t start,
                                                      uint32_t end) const {
  std::vector<std::shared_ptr<Packet>> packets;
  if (start > end)
    return packets;
  uv_rwlock_rdlock(&d->rwlock);
  for (auto it = d->packets.find(start); it != d->packets.end(); ++it) {
    packets.push_back(it->second);
    if (it->second->seq() >= end)
      break;
  }
  uv_rwlock_rdunlock(&d->rwlock);
  return packets;
}

std::shared_ptr<Packet> PacketStore::get(uint32_t seq) const {
  std::shared_ptr<Packet> pkt;
  uv_rwlock_rdlock(&d->rwlock);
  auto it = d->packets.find(seq);
  if (it != d->packets.end())
    pkt = it->second;
  uv_rwlock_rdunlock(&d->rwlock);
  return pkt;
}

uint32_t PacketStore::maxSeq() const { return d->maxSeq; }

int PacketStore::addHandler(const std::function<void(uint32_t)> &cb) {
  static int handlerId = 0;
  int id = ++handlerId;
  uv_rwlock_wrlock(&d->rwlock);
  d->handlers[id] = cb;
  uv_rwlock_wrunlock(&d->rwlock);
  return id;
}

void PacketStore::removeHandler(int id) {
  uv_rwlock_wrlock(&d->rwlock);
  d->handlers.erase(id);
  uv_rwlock_wrunlock(&d->rwlock);
}

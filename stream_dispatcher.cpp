#include "stream_dispatcher.hpp"
#include "stream_chunk.hpp"
#include "stream_dissector_thread.hpp"
#include <chrono>
#include <map>
#include <mutex>
#include <random>
#include <unordered_map>

namespace {
struct Stream {
  int thread = -1;
  std::chrono::time_point<std::chrono::system_clock> lastUsed =
      std::chrono::system_clock::now();
};
}

class StreamDispatcher::Private {
public:
  Private(const std::shared_ptr<Context> &ctx);

public:
  std::shared_ptr<Context> ctx;
  std::mutex mutex;
  std::vector<std::unique_ptr<StreamDissectorThread>> dissectorThreads;
  std::map<uint32_t, std::vector<std::unique_ptr<StreamChunk>>> streamChunks;
  std::unordered_map<std::string, Stream> streams;
  uint32_t maxSeq = 0;
};

StreamDispatcher::Private::Private(const std::shared_ptr<Context> &ctx)
    : ctx(ctx) {

  auto dissCtx = std::make_shared<StreamDissectorThread::Context>();
  dissCtx->tmpDir = ctx->tmpDir;
  dissCtx->streamsCb = ctx->streamsCb;
  dissCtx->errorCb = ctx->errorCb;
  dissCtx->dissectors = ctx->dissectors;
  for (int i = 0; i < 4; ++i) {
    dissectorThreads.emplace_back(new StreamDissectorThread(dissCtx));
  }
}

StreamDispatcher::StreamDispatcher(const std::shared_ptr<Context> &ctx)
    : d(std::make_shared<Private>(ctx)) {}

StreamDispatcher::~StreamDispatcher() {}

void StreamDispatcher::insert(
    uint32_t seq, std::vector<std::unique_ptr<StreamChunk>> streamChunks) {
  std::lock_guard<std::mutex> lock(d->mutex);
  d->streamChunks[seq] = std::move(streamChunks);

  auto it = d->streamChunks.begin();
  for (; it != d->streamChunks.end() && it->first == d->maxSeq + 1;
       d->maxSeq++, ++it) {
    for (auto &chunk : it->second) {
      const std::string &id = chunk->id();
      Stream &stream = d->streams[id];
      if (stream.thread < 0) {
        static std::random_device dev;
        static std::mt19937_64 generator(dev());
        static std::uniform_int_distribution<int> dist(
            0, d->dissectorThreads.size() - 1);
        stream.thread = 0;//dist(generator);
      }
      stream.lastUsed = std::chrono::system_clock::now();
      StreamDissectorThread &thread = *d->dissectorThreads[stream.thread];
      thread.insert(std::move(chunk));
    }
  }
  d->streamChunks.erase(d->streamChunks.begin(), it);
}

void StreamDispatcher::insert(
    std::vector<std::unique_ptr<StreamChunk>> streamChunks) {
  std::lock_guard<std::mutex> lock(d->mutex);
  for (auto &chunk : streamChunks) {
    const std::string &id = chunk->id();
    Stream &stream = d->streams[id];
    if (stream.thread < 0) {
      static std::random_device dev;
      static std::mt19937_64 generator(dev());
      static std::uniform_int_distribution<int> dist(
          0, d->dissectorThreads.size() - 1);
      stream.thread = dist(generator);
    }
    stream.lastUsed = std::chrono::system_clock::now();
    StreamDissectorThread &thread = *d->dissectorThreads[stream.thread];
    thread.insert(std::move(chunk));
  }
}

#include "large_buffer.hpp"
#include <cstdlib>
#include <sstream>
#include <random>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>

namespace {
std::string randomId() {
  std::random_device dev;
  std::mt19937_64 generator(dev());
  std::uniform_int_distribution<int> dist(0, 255);
  std::stringstream stream;
  for (size_t i = 0; i < 16; ++i) {
    stream << std::hex << std::setfill('0') << std::setw(2) << dist(generator);
  }
  return stream.str();
}
}

std::string LargeBuffer::tmpDir() {
  atexit([]() { printf("%s\n", "atexit"); });
  static std::string path;
  if (path.empty()) {
    path = "/tmp";
    const char *envs[] = {"TMP", "TEMP", "TMPDIR", "TEMPDIR"};
    for (const char *env : envs) {
      const char *tmp = std::getenv(env);
      if (tmp && strlen(tmp)) {
        path = tmp;
        break;
      }
    }
    path += "/nylonfilter_" + randomId();
    mkdir(path.c_str(), 0755);
  }
  return path;
}

class LargeBuffer::Private {
public:
  std::string id;
};

LargeBuffer::LargeBuffer(const std::string &id) : d(new Private()) {
  if (id.empty()) {
    d->id = randomId();
  } else {
    d->id = id;
  }
}

LargeBuffer::~LargeBuffer() {}

std::string LargeBuffer::id() const { return d->id; }

void LargeBuffer::write(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void LargeBuffer::read(const v8::FunctionCallbackInfo<v8::Value> &args) {}

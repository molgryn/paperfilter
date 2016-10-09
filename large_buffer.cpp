#include "large_buffer.hpp"
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

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

std::string getTmpDir() {
  std::string path = "/tmp";
  const char *envs[] = {"TMP", "TEMP", "TMPDIR", "TEMPDIR"};
  for (const char *env : envs) {
    const char *tmp = std::getenv(env);
    if (tmp && strlen(tmp)) {
      path = tmp;
      break;
    }
  }
  path += "/paperfilter_" + randomId();
#ifdef _WIN32
  _mkdir(path.c_str());
#else
  mkdir(path.c_str(), 0755);
#endif
  return path;
}
}

std::string LargeBuffer::tmpDir() {
  static const std::string path = getTmpDir();
  return path;
}

class LargeBuffer::Private {
public:
  std::string id = randomId();
};

LargeBuffer::LargeBuffer() : d(std::make_shared<Private>()) {}

LargeBuffer::LargeBuffer(const LargeBuffer &other)
    : d(std::make_shared<Private>()) {
  *this = other;
}

LargeBuffer &LargeBuffer::operator=(const LargeBuffer &other) {
  if (&other == this)
    return *this;
  d->id = other.d->id;
  return *this;
}

LargeBuffer::~LargeBuffer() {}

std::string LargeBuffer::id() const { return d->id; }

std::string LargeBuffer::path() const { return tmpDir() + "/" + d->id; }

void LargeBuffer::write(const v8::FunctionCallbackInfo<v8::Value> &args) {}

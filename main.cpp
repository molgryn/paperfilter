#include "session_layer_wrapper.hpp"
#include "session_packet_wrapper.hpp"
#include "session_wrapper.hpp"
#include <nan.h>

#include <signal.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

using namespace v8;

void Init(v8::Local<v8::Object> exports) {
  signal(SIGSEGV, [](int sig) {
    void *array[256];
    size_t size = backtrace(array, 256);
    //auto spd = spdlog::get("console");
    //spd->critical("Error: signal {}", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
});
std::set_terminate([]() {
    void *array[256];
    size_t size = backtrace(array, 256);
    //auto spd = spdlog::get("console");
    //spd->critical("Error: std::terminate");
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
});

  SessionPacketWrapper::Init(exports);
  SessionLayerWrapper::Init(exports);
  SessionWrapper::Init(exports);
}

NODE_MODULE(hello, Init)

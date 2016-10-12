#include "session_item_value_wrapper.hpp"
#include "session_item_wrapper.hpp"
#include "session_large_buffer_wrapper.hpp"
#include "session_layer_wrapper.hpp"
#include "session_packet_wrapper.hpp"
#include "session_wrapper.hpp"
#include <nan.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <execinfo.h>

using namespace v8;

void Init(v8::Local<v8::Object> exports) {
  signal(SIGSEGV, [](int sig) {
    void *array[256];
    size_t size = backtrace(array, 256);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
});
std::set_terminate([]() {
    void *array[256];
    size_t size = backtrace(array, 256);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
});
  SessionPacketWrapper::Init(exports);
  SessionLayerWrapper::Init(exports);
  SessionItemWrapper::Init(exports);
  SessionItemValueWrapper::Init(exports);
  SessionLargeBufferWrapper::Init(exports);
  SessionWrapper::Init(exports);
}

NODE_MODULE(paperfilter, Init)

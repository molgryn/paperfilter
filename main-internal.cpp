#include "paper_context.hpp"
#include <nan.h>

void Init(v8::Local<v8::Object> exports) { PaperContext::init(exports); }

NODE_MODULE(paperfilter_internal, Init)

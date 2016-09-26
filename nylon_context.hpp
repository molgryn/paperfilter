#ifndef NYLON_CONTEXT_HPP
#define NYLON_CONTEXT_HPP

#include <v8.h>

class NylonContext {
public:
  static void init(v8::Isolate *isolate);
};

#endif

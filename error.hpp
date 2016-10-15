#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <v8.h>

std::string messageToJson(v8::Local<v8::Message> msg);

#endif

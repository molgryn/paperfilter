#ifndef ERROR_HPP
#define ERROR_HPP

#include <v8.h>
#include <string>

std::string messageToJson(v8::Local<v8::Message> msg);

#endif

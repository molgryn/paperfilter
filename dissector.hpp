#ifndef DISSECTOR_HPP
#define DISSECTOR_HPP

#include <v8.h>
#include <vector>
#include <string>
#include <regex>

struct Dissector {
public:
  explicit Dissector(v8::Local<v8::Object> option);

public:
  std::vector<std::string> namespaces;
  std::vector<std::regex> regexNamespaces;
  std::string script;
};

#endif

#include "error.hpp"
#include <json11.hpp>
#include <v8pp/class.hpp>

std::string messageToJson(v8::Local<v8::Message> msg) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  if (!msg.IsEmpty()) {
    json11::Json error = json11::Json::object{
        {"message", v8pp::from_v8<std::string>(isolate, msg->Get(), "")},
        {"resourceName",
         v8pp::from_v8<std::string>(isolate, msg->GetScriptResourceName(), "")},
        {"source",
         v8pp::from_v8<std::string>(isolate, msg->GetSourceLine(), "")},
        {"lineNumber", msg->GetLineNumber()},
        {"startPosition", msg->GetStartPosition()},
        {"endPosition", msg->GetEndPosition()},
        {"startColumn", msg->GetStartColumn()},
        {"endColumn", msg->GetEndColumn()},
        {"domain", "dissector"}};
    return error.dump();
  } else {
    json11::Json error = json11::Json::object{};
    return error.dump();
  }
}

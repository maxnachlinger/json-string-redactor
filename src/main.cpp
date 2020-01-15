#include "./include/rapidjson/reader.h"
#include "./include/rapidjson/writer.h"
#include "./include/rapidjson/filereadstream.h"
#include "./include/rapidjson/filewritestream.h"
#include "./include/rapidjson/error/en.h"
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <iterator>
#include <napi.h>

using namespace rapidjson;

// This handler forwards event into an output handler

template <typename OutputHandler>
class FilterKeyHandler {
public:
    typedef char Ch;

    FilterKeyHandler(OutputHandler& outputHandler, const std::set<std::string>* keysToRedact) :
        outputHandler_(outputHandler), keysToRedact_(keysToRedact), buffer_(), redact_()
    {}

    bool Null() { redact_ = false; return outputHandler_.Null(); }
    bool Bool(bool b) { redact_ = false; return outputHandler_.Bool(b); }
    bool Int(int i) { redact_ = false; return outputHandler_.Int(i); }
    bool Uint(unsigned u) { redact_ = false; return outputHandler_.Uint(u); }
    bool Int64(int64_t i) { redact_ = false; return outputHandler_.Int64(i); }
    bool Uint64(uint64_t u) { redact_ = false; return outputHandler_.Uint64(u); }
    bool Double(double d) { redact_ = false; return outputHandler_.Double(d); }
    bool RawNumber(const char* str, SizeType length, bool copy) { return outputHandler_.RawNumber(str, length, copy); }
    bool StartObject() { redact_ = false; return outputHandler_.StartObject(); }
    bool EndObject(SizeType memberCount) { return outputHandler_.EndObject(memberCount); }
    bool StartArray() { redact_ = false; return outputHandler_.StartArray(); }
    bool EndArray(SizeType elementCount) { return outputHandler_.EndArray(elementCount); }

    // if we're redacting the current key, replace the string value with an equivalent amount of *'s
    // e.g. redacting "name" key: -> "test" -> "****"
    bool String(const Ch* str, SizeType len, bool copy) {
        if (redact_ == false) {
            return outputHandler_.String(str, len, copy);
        }

        redact_ = false;

        buffer_.clear();
        for (SizeType i = 0; i < len; i++) {
            buffer_.push_back('*');
        }
        return outputHandler_.String(&buffer_.front(), len, true);
    }

    bool Key(const Ch* str, SizeType len, bool copy) {
        // we'll redact if this key is in our set of keysToRedact_
        redact_ = keysToRedact_->find(str) != keysToRedact_->end();
        return outputHandler_.Key(str, len, copy);
    }

    std::vector<char> buffer_;
    bool redact_;

private:
    FilterKeyHandler(const FilterKeyHandler&);
    FilterKeyHandler& operator=(const FilterKeyHandler&);
    OutputHandler& outputHandler_;
    const std::set<std::string>* keysToRedact_;
};

Napi::String redactJSONKeyStringValues(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 2 || !info[0].IsArray() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Expected an array of property names to redact and a JSON string.")
      .ThrowAsJavaScriptException();
  }

  // fill keysToRedact
  Napi::Array keysToRedactArray = info[0].As<Napi::Array>();
  unsigned int length = keysToRedactArray.Length();

  std::set<std::string> keysToRedact;

  for (unsigned int i = 0; i < length; i++) {
    keysToRedact.insert(keysToRedactArray.Get(i).ToString());
  }

  std::string jsonString = info[1].As<Napi::String>();

  Reader reader;
  StringStream inputStream(jsonString.c_str());

  StringBuffer outputStream;
  Writer<StringBuffer> writer(outputStream);

  FilterKeyHandler<Writer<StringBuffer> > filter(writer, &keysToRedact);
  if (!reader.Parse(inputStream, filter)) {
    Napi::TypeError::New(env, "Could not parse JSON.")
      .ThrowAsJavaScriptException();
  }

  return Napi::String::New(env, outputStream.GetString());
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "redactJSONKeyStringValues"), Napi::Function::New(env, redactJSONKeyStringValues));
  return exports;
}

NODE_API_MODULE(jsonStringRedactor, init);

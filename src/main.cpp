#include "./include/rapidjson/reader.h"
#include "./include/rapidjson/writer.h"
#include "./include/rapidjson/filereadstream.h"
#include "./include/rapidjson/filewritestream.h"
#include "./include/rapidjson/error/en.h"
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <iterator>
#include <napi.h>

using namespace rapidjson;
using namespace std;

// This handler forwards event into an output handler
template<typename OutputHandler>
struct FilterKeyHandler {
    FilterKeyHandler(OutputHandler& outputHandler, unordered_set<string>* keysToRedact) :
        out_(outputHandler), keysToRedact_(keysToRedact), buffer_(), redact_()
    {}
    typedef char Ch;

    bool Null() { return out_.Null(); }
    bool Bool(bool b) { return out_.Bool(b); }
    bool Int(int i) { return out_.Int(i); }
    bool Uint(unsigned u) { return out_.Uint(u); }
    bool Int64(int64_t i) { return out_.Int64(i); }
    bool Uint64(uint64_t u) { return out_.Uint64(u); }
    bool Double(double d) { return out_.Double(d); }

    // if we're redacting the current key, replace the string value with an equivalent amount of *'s
    // e.g. redacting "name" key: -> "test" -> "****"
    bool String(const Ch* str, SizeType len, bool copy) {
        if (redact_ == false) {
            return out_.String(str, len, copy);
        }

        redact_ = false;

        buffer_.clear();
        for (SizeType i = 0; i < len; i++) {
            buffer_.push_back('*');
        }
        return out_.String(&buffer_.front(), len, true);
    }

    bool Key(const Ch* str, SizeType len, bool copy) {
        // is this key in our set to redact?
        redact_ = keysToRedact_->find(str) != keysToRedact_->end();
        return out_.Key(str, len, copy);
    }

    bool StartObject() { return out_.StartObject(); }
    bool EndObject(SizeType memberCount) { return out_.EndObject(memberCount); }
    bool StartArray() { return out_.StartArray(); }
    bool EndArray(SizeType elementCount) { return out_.EndArray(elementCount); }
    bool RawNumber (const Ch *str, SizeType length, bool copy=false)  { return out_.RawNumber(str, length, copy); }

    OutputHandler& out_;
    unordered_set<string>* keysToRedact_;
    vector<char> buffer_;
    bool redact_;
};

Napi::String redactJSONKeyStringValues(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 2 || !info[0].IsArray() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Expected an array of property names to redact and a JSON string.")
      .ThrowAsJavaScriptException();
  }

  unordered_set<string> keysToRedact;
  Napi::Array keysToRedactArray = info[0].As<Napi::Array>();
  unsigned int length = keysToRedactArray.Length();

  for (unsigned int i = 0; i < length; i++) {
    keysToRedact.insert(keysToRedactArray.Get(i).ToString());
  }

  std::string jsonString = info[1].As<Napi::String>();

  Reader reader;
  StringStream inputStream(jsonString.c_str());

  StringBuffer outputStream;
  Writer<StringBuffer> writer(outputStream);

  FilterKeyHandler<Writer<StringBuffer>> filter(writer, &keysToRedact);
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

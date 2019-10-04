#include "utf8streams.hpp"

namespace utf8streams {

UTF8IStream::UTF8IStream(std::istream &&stream, Encoding sourceEncoding)
    : buf(nullptr) {
  (void)stream;
  (void)sourceEncoding;
}

UTF8IStream::~UTF8IStream() { delete buf; }

Encoding UTF8IStream::getEncoding() const { return Encoding::Unknown; }

} // namespace utf8streams

#pragma once
#include <istream>

namespace utf8streams {

enum class Encoding { Unknown, Utf8, Utf16LE, Utf16BE, Utf32LE, Utf32BE };

class UTF8IStream : public std::istream {
private:
  std::streambuf *buf;

public:
  explicit UTF8IStream(std::istream &&stream,
                       Encoding sourceEncoding = Encoding::Unknown);

  UTF8IStream(const UTF8IStream &) = delete;

  ~UTF8IStream() override;

  Encoding getEncoding() const;
};

} // namespace utf8streams

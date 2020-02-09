#pragma once
#include <istream>

namespace utf8streams {

enum class Encoding { Unknown, Utf8, Utf16LE, Utf16BE, Utf32LE, Utf32BE };

Encoding guessEncoding(std::istream &stream);

class Error : public std::runtime_error {
public:
  explicit Error(const std::string &message);
};

class UnicodeError : public Error {
public:
  explicit UnicodeError(const std::string &message);
};

class UTF8StreamBuf : public std::streambuf {
private:
  struct RingBuffer {
  private:
    uint8_t data[4];
    uint8_t pos;
    uint8_t filled;

  public:
    RingBuffer();

    void put(uint8_t byte);

    uint8_t get();

    uint8_t peek();

    bool hasData() const;
  };

  typedef std::streamsize (UTF8StreamBuf::*XsgetnCallback)(char *buffer,
                                                           std::streamsize n);
  typedef int (UTF8StreamBuf::*UnderflowCallback)();
  typedef int (UTF8StreamBuf::*UflowCallback)();

  std::streambuf *originalBuf;
  XsgetnCallback xsgetnCallback;
  UnderflowCallback underflowCallback;
  UflowCallback uflowCallback;
  RingBuffer byteBuff;

  void putUnicode(uint32_t unicode);

  std::streamsize xsgetnUtf8(char *buffer, std::streamsize n);

  int underflowUtf8();

  int uflowUtf8();

  std::streamsize xsgetnUtf16LE(char *buffer, std::streamsize n);

  int underflowUtf16LE();

  int uflowUtf16LE();

  std::streamsize xsgetnUtf16BE(char *buffer, std::streamsize n);

  int underflowUtf16BE();

  int uflowUtf16BE();

  std::streamsize xsgetnUtf32LE(char *buffer, std::streamsize n);

  int underflowUtf32LE();

  int uflowUtf32LE();

  std::streamsize xsgetnUtf32BE(char *buffer, std::streamsize n);

  int underflowUtf32BE();

  int uflowUtf32BE();

protected:
  int sync() override;

  std::streamsize showmanyc() override;

  std::streamsize xsgetn(char *buffer, std::streamsize n) override;

  int underflow() override;

  int uflow() override;

public:
  explicit UTF8StreamBuf(std::istream &stream, Encoding sourceEncoding);
};

} // namespace utf8streams

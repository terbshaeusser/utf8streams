#include "utf8streams.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include <string>
#include <tuple>

namespace utf8streams {

static uint16_t swap16(uint16_t n) {
  return (static_cast<uint32_t>(n & 0xFF00u) >> 8u) |
         (static_cast<uint32_t>(n & 0xFFu) << 8u);
}

static uint16_t fromLE16(uint16_t n) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return n;
#else
  return swap16(n);
#endif
}

static uint16_t fromBE16(uint16_t n) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return swap16(n);
#else
  return n;
#endif
}

static uint32_t swap32(uint32_t n) {
  return (n >> 24u) | ((n & 0xFF0000u) >> 8u) | ((n & 0xFF00u) << 8u) |
         (n << 24u);
}

static uint32_t fromLE32(uint32_t n) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return n;
#else
  return swap32(n);
#endif
}

static uint32_t fromBE32(uint32_t n) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return swap32(n);
#else
  return n;
#endif
}

static bool isHighSurrogate(uint32_t codePoint) {
  return codePoint >= 0xD800 && codePoint <= 0xDBFF;
}

static bool isLowSurrogate(uint32_t codePoint) {
  return codePoint >= 0xDC00 && codePoint <= 0xDFFF;
}

[[noreturn]] static void unreachable() {
  throw std::runtime_error("Unreachable code reached");
}

constexpr char UTF8_BOM[] = {'\xEF', '\xBB', '\xBF'};
constexpr char UTF16LE_BOM[] = {'\xFF', '\xFE'};
constexpr char UTF16BE_BOM[] = {'\xFE', '\xFF'};
constexpr char UTF32LE_BOM[] = {'\xFF', '\xFE', '\x00', '\x00'};
constexpr char UTF32BE_BOM[] = {'\x00', '\x00', '\xFE', '\xFF'};

using EncodingInfo = std::tuple<Encoding, size_t, const void *>;

static const EncodingInfo UTF8_INFO =
    EncodingInfo(Encoding::Utf8, sizeof(UTF8_BOM), UTF8_BOM);
static const EncodingInfo UTF16LE_INFO =
    EncodingInfo(Encoding::Utf16LE, sizeof(UTF16LE_BOM), UTF16LE_BOM);
static const EncodingInfo UTF16BE_INFO =
    EncodingInfo(Encoding::Utf16BE, sizeof(UTF16BE_BOM), UTF16BE_BOM);
static const EncodingInfo UTF32LE_INFO =
    EncodingInfo(Encoding::Utf32LE, sizeof(UTF32LE_BOM), UTF32LE_BOM);
static const EncodingInfo UTF32BE_INFO =
    EncodingInfo(Encoding::Utf32BE, sizeof(UTF32BE_BOM), UTF32BE_BOM);

Encoding guessEncoding(std::istream &stream) {
  uint8_t bom[4];

  stream.read(reinterpret_cast<char *>(&bom[0]), sizeof(bom));
  auto readBytes = static_cast<size_t>(stream.gcount());

  for (auto encodingInfo : std::initializer_list<EncodingInfo>{
           UTF32LE_INFO, UTF32BE_INFO, UTF16LE_INFO, UTF16BE_INFO, UTF8_INFO}) {
    auto encoding = std::get<0>(encodingInfo);
    auto len = std::get<1>(encodingInfo);
    auto data = std::get<2>(encodingInfo);

    if (readBytes >= len && std::memcmp(bom, data, len) == 0) {
      if (len != readBytes) {
        stream.seekg(-static_cast<std::streampos>(readBytes - len),
                     std::ios::cur);
      }

      return encoding;
    }
  }

  stream.seekg(-static_cast<std::streamoff>(readBytes), std::ios::cur);
  return Encoding::Unknown;
}

Error::Error(const std::string &message) : runtime_error(message) {}

UnicodeError::UnicodeError(const std::string &message) : Error(message) {}

UTF8StreamBuf::RingBuffer::RingBuffer() : pos(0), filled(0) {}

void UTF8StreamBuf::RingBuffer::put(uint8_t byte) {
  assert(filled < sizeof(data));

  data[(pos + filled) % sizeof(data)] = byte;
  ++filled;
}

uint8_t UTF8StreamBuf::RingBuffer::get() {
  assert(filled > 0);

  auto byte = data[pos];
  pos = (pos + 1) % sizeof(data);
  --filled;

  return byte;
}

uint8_t UTF8StreamBuf::RingBuffer::peek() {
  assert(filled > 0);

  return data[pos];
}

bool UTF8StreamBuf::RingBuffer::hasData() const { return filled != 0; }

void UTF8StreamBuf::putUnicode(uint32_t unicode) {
  if (unicode < 0x80) {
    byteBuff.put(static_cast<uint8_t>(unicode));
  } else if (unicode < 0x800) {
    byteBuff.put(static_cast<uint8_t>(0xC0u | (unicode >> 6u)));
    byteBuff.put(static_cast<uint8_t>(0x80u | (unicode & 0x3Fu)));
  } else if (unicode < 0x10000) {
    byteBuff.put(static_cast<uint8_t>(0xE0u | (unicode >> 12u)));
    byteBuff.put(static_cast<uint8_t>(0x80u | ((unicode >> 6u) & 0x3Fu)));
    byteBuff.put(static_cast<uint8_t>(0x80u | (unicode & 0x3Fu)));
  } else if (unicode < 0x10FFFF) {
    byteBuff.put(static_cast<uint8_t>(0xF0u | (unicode >> 18u)));
    byteBuff.put(static_cast<uint8_t>(0x80u | ((unicode >> 12u) & 0x3Fu)));
    byteBuff.put(static_cast<uint8_t>(0x80u | ((unicode >> 6u) & 0x3Fu)));
    byteBuff.put(static_cast<uint8_t>(0x80u | (unicode & 0x3Fu)));
  } else {
    throw UnicodeError("Invalid Unicode sign " + std::to_string(unicode));
  }
}

std::streamsize UTF8StreamBuf::xsgetnUtf8(char *buffer, std::streamsize n) {
  return originalBuf->sgetn(buffer, n);
}

int UTF8StreamBuf::underflowUtf8() { return originalBuf->sgetc(); }

int UTF8StreamBuf::uflowUtf8() { return originalBuf->snextc(); }

std::streamsize UTF8StreamBuf::xsgetnUtf16LE(char *buffer, std::streamsize n) {
  auto readBytes = 0;

  while (n > 0) {
    auto c = uflowUtf16LE();
    if (c == EOF) {
      break;
    }
    *buffer = static_cast<char>(c);

    ++buffer;
    --n;
    ++readBytes;
  }

  return readBytes;
}

int UTF8StreamBuf::underflowUtf16LE() {
  if (byteBuff.hasData()) {
    return byteBuff.peek();
  }

  uint16_t codePoint = 0;
  auto readBytes = originalBuf->sgetn(reinterpret_cast<char *>(&codePoint),
                                      sizeof(codePoint));

  if (readBytes == 0) {
    return EOF;
  }
  if (readBytes != sizeof(codePoint)) {
    throw UnicodeError("Incomplete code point found");
  }

  codePoint = fromLE16(codePoint);

  uint32_t unicode = codePoint;
  if (isHighSurrogate(codePoint)) {
    uint16_t codePoint2 = 0;
    readBytes = originalBuf->sgetn(reinterpret_cast<char *>(&codePoint2),
                                   sizeof(codePoint2));

    if (readBytes == 0) {
      throw UnicodeError(
          "High surrogate found without following low surrogate");
    }
    if (readBytes != sizeof(codePoint)) {
      throw UnicodeError("Incomplete code point found");
    }

    codePoint2 = fromLE16(codePoint2);

    if (!isLowSurrogate(codePoint2)) {
      throw UnicodeError(
          "High surrogate found without following low surrogate");
    }

    unicode = 0x10000 + ((static_cast<uint32_t>(codePoint - 0xD800) << 10u) |
                         (static_cast<uint32_t>(codePoint2 - 0xDC00)));
  } else if (isLowSurrogate(codePoint)) {
    throw UnicodeError("Low surrogate found without leading high surrogate");
  }

  putUnicode(unicode);

  return byteBuff.peek();
}

int UTF8StreamBuf::uflowUtf16LE() {
  underflowUtf16LE();
  if (!byteBuff.hasData()) {
    return EOF;
  }
  return byteBuff.get();
}

std::streamsize UTF8StreamBuf::xsgetnUtf16BE(char *buffer, std::streamsize n) {
  auto readBytes = 0;

  while (n > 0) {
    auto c = uflowUtf16BE();
    if (c == EOF) {
      break;
    }
    *buffer = static_cast<char>(c);

    ++buffer;
    --n;
    ++readBytes;
  }

  return readBytes;
}

int UTF8StreamBuf::underflowUtf16BE() {
  if (byteBuff.hasData()) {
    return byteBuff.peek();
  }

  uint16_t codePoint = 0;
  auto readBytes = originalBuf->sgetn(reinterpret_cast<char *>(&codePoint),
                                      sizeof(codePoint));

  if (readBytes == 0) {
    return EOF;
  }
  if (readBytes != sizeof(codePoint)) {
    throw UnicodeError("Incomplete code point found");
  }

  codePoint = fromBE16(codePoint);

  uint32_t unicode = codePoint;
  if (isHighSurrogate(codePoint)) {
    uint16_t codePoint2 = 0;
    readBytes = originalBuf->sgetn(reinterpret_cast<char *>(&codePoint2),
                                   sizeof(codePoint2));

    if (readBytes == 0) {
      throw UnicodeError(
          "High surrogate found without following low surrogate");
    }
    if (readBytes != sizeof(codePoint)) {
      throw UnicodeError("Incomplete code point found");
    }

    codePoint2 = fromBE16(codePoint2);

    if (!isLowSurrogate(codePoint2)) {
      throw UnicodeError(
          "High surrogate found without following low surrogate");
    }

    unicode = 0x10000 + ((static_cast<uint32_t>(codePoint - 0xD800) << 10u) |
                         (static_cast<uint32_t>(codePoint2 - 0xDC00)));
  } else if (isLowSurrogate(codePoint)) {
    throw UnicodeError("Low surrogate found without leading high surrogate");
  }

  putUnicode(unicode);

  return byteBuff.peek();
}

int UTF8StreamBuf::uflowUtf16BE() {
  underflowUtf16BE();
  if (!byteBuff.hasData()) {
    return EOF;
  }
  return byteBuff.get();
}

std::streamsize UTF8StreamBuf::xsgetnUtf32LE(char *buffer, std::streamsize n) {
  auto readBytes = 0;

  while (n > 0) {
    auto c = uflowUtf32LE();
    if (c == EOF) {
      break;
    }
    *buffer = static_cast<char>(c);

    ++buffer;
    --n;
    ++readBytes;
  }

  return readBytes;
}

int UTF8StreamBuf::underflowUtf32LE() {
  if (byteBuff.hasData()) {
    return byteBuff.peek();
  }

  uint32_t unicode = 0;
  auto readBytes =
      originalBuf->sgetn(reinterpret_cast<char *>(&unicode), sizeof(unicode));

  if (readBytes == 0) {
    return EOF;
  }
  if (readBytes != sizeof(unicode)) {
    throw UnicodeError("Incomplete code point found");
  }

  unicode = fromLE32(unicode);
  putUnicode(unicode);

  return byteBuff.peek();
}

int UTF8StreamBuf::uflowUtf32LE() {
  underflowUtf32LE();
  if (!byteBuff.hasData()) {
    return EOF;
  }
  return byteBuff.get();
}

std::streamsize UTF8StreamBuf::xsgetnUtf32BE(char *buffer, std::streamsize n) {
  auto readBytes = 0;

  while (n > 0) {
    auto c = uflowUtf32BE();
    if (c == EOF) {
      break;
    }
    *buffer = static_cast<char>(c);

    ++buffer;
    --n;
    ++readBytes;
  }

  return readBytes;
}

int UTF8StreamBuf::underflowUtf32BE() {
  if (byteBuff.hasData()) {
    return byteBuff.peek();
  }

  uint32_t unicode = 0;
  auto readBytes =
      originalBuf->sgetn(reinterpret_cast<char *>(&unicode), sizeof(unicode));

  if (readBytes == 0) {
    return EOF;
  }
  if (readBytes != sizeof(unicode)) {
    throw UnicodeError("Incomplete code point found");
  }

  unicode = fromBE32(unicode);
  putUnicode(unicode);

  return byteBuff.peek();
}

int UTF8StreamBuf::uflowUtf32BE() {
  underflowUtf32BE();
  if (!byteBuff.hasData()) {
    return EOF;
  }
  return byteBuff.get();
}

int UTF8StreamBuf::sync() { return originalBuf->pubsync(); }

std::streamsize UTF8StreamBuf::showmanyc() {
  auto available = originalBuf->in_avail();
  return available > 0 ? std::max<std::streamsize>(
                             1, static_cast<std::streamsize>(available) / 4)
                       : available;
}

std::streamsize UTF8StreamBuf::xsgetn(char *buffer, std::streamsize n) {
  return xsgetnCallback(this, buffer, n);
}

int UTF8StreamBuf::underflow() { return underflowCallback(this); }

int UTF8StreamBuf::uflow() { return uflowCallback(this); }

UTF8StreamBuf::UTF8StreamBuf(std::istream &stream, Encoding sourceEncoding)
    : originalBuf(stream.rdbuf()) {
  stream.rdbuf(this);

  if (originalBuf == nullptr) {
    throw Error("Buffer of stream is not set");
  }

  switch (sourceEncoding) {
  case Encoding::Unknown:
    throw Error("Cannot create UTF8StreamBuf with unknown encoding");
  case Encoding::Utf8: {
    auto f = &UTF8StreamBuf::xsgetnUtf8;
    xsgetnCallback = *reinterpret_cast<XsgetnCallback *>(&f);
    auto f2 = &UTF8StreamBuf::underflowUtf8;
    underflowCallback = *reinterpret_cast<UnderflowCallback *>(&f2);
    auto f3 = &UTF8StreamBuf::uflowUtf8;
    uflowCallback = *reinterpret_cast<UflowCallback *>(&f3);
    break;
  }
  case Encoding::Utf16LE: {
    auto f = &UTF8StreamBuf::xsgetnUtf16LE;
    xsgetnCallback = *reinterpret_cast<XsgetnCallback *>(&f);
    auto f2 = &UTF8StreamBuf::underflowUtf16LE;
    underflowCallback = *reinterpret_cast<UnderflowCallback *>(&f2);
    auto f3 = &UTF8StreamBuf::uflowUtf16LE;
    uflowCallback = *reinterpret_cast<UflowCallback *>(&f3);
    break;
  }
  case Encoding::Utf16BE: {
    auto f = &UTF8StreamBuf::xsgetnUtf16BE;
    xsgetnCallback = *reinterpret_cast<XsgetnCallback *>(&f);
    auto f2 = &UTF8StreamBuf::underflowUtf16BE;
    underflowCallback = *reinterpret_cast<UnderflowCallback *>(&f2);
    auto f3 = &UTF8StreamBuf::uflowUtf16BE;
    uflowCallback = *reinterpret_cast<UflowCallback *>(&f3);
    break;
  }
  case Encoding::Utf32LE: {
    auto f = &UTF8StreamBuf::xsgetnUtf32LE;
    xsgetnCallback = *reinterpret_cast<XsgetnCallback *>(&f);
    auto f2 = &UTF8StreamBuf::underflowUtf32LE;
    underflowCallback = *reinterpret_cast<UnderflowCallback *>(&f2);
    auto f3 = &UTF8StreamBuf::uflowUtf32LE;
    uflowCallback = *reinterpret_cast<UflowCallback *>(&f3);
    break;
  }
  case Encoding::Utf32BE: {
    auto f = &UTF8StreamBuf::xsgetnUtf32BE;
    xsgetnCallback = *reinterpret_cast<XsgetnCallback *>(&f);
    auto f2 = &UTF8StreamBuf::underflowUtf32BE;
    underflowCallback = *reinterpret_cast<UnderflowCallback *>(&f2);
    auto f3 = &UTF8StreamBuf::uflowUtf32BE;
    uflowCallback = *reinterpret_cast<UflowCallback *>(&f3);
    break;
  }
  default:
    unreachable();
  }
}

} // namespace utf8streams

#include <cstring>
#include <gtest/gtest.h>
#include <utf8streams.hpp>

TEST(guessEncoding, noBOMShort) {
  std::istringstream stream("0");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Unknown, encoding);
  EXPECT_EQ(0, stream.tellg());
}

TEST(guessEncoding, noBOM) {
  std::istringstream stream("Hello World");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Unknown, encoding);
  EXPECT_EQ(0, stream.tellg());
}

TEST(guessEncoding, utf8BOM) {
  std::istringstream stream("\xEF\xBB\xBFHello World");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf8, encoding);
  EXPECT_EQ(3, stream.tellg());
}

TEST(guessEncoding, utf16LEBOM) {
  std::istringstream stream("\xFF\xFEHello World");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf16LE, encoding);
  EXPECT_EQ(2, stream.tellg());
}

TEST(guessEncoding, utf16LEBOMEmpty) {
  std::istringstream stream("\xFF\xFE");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf16LE, encoding);
  EXPECT_EQ(2, stream.tellg());
}

TEST(guessEncoding, utf16BEBOM) {
  std::istringstream stream("\xFE\xFFHello World");
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf16BE, encoding);
  EXPECT_EQ(2, stream.tellg());
}

TEST(guessEncoding, utf32LEBOM) {
  std::istringstream stream(std::string("\xFF\xFE\x00\x00Hello World", 15));
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf32LE, encoding);
  EXPECT_EQ(4, stream.tellg());
}

TEST(guessEncoding, utf32BEBOM) {
  std::istringstream stream(std::string("\x00\x00\xFE\xFFHello World", 15));
  auto encoding = utf8streams::guessEncoding(stream);

  EXPECT_EQ(utf8streams::Encoding::Utf32BE, encoding);
  EXPECT_EQ(4, stream.tellg());
}

TEST(Utf8, ascii) {
  std::istringstream stream("Hello World");
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

TEST(Utf8, asciiGet) {
  std::string content = "Hello World";
  std::istringstream stream(content);
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  for (auto c : content) {
    EXPECT_EQ(c, stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf8, multiByte) {
  std::istringstream stream("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E");
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0,
            std::memcmp("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E", buffer, 11));
}

TEST(Utf8, multiByteParts) {
  std::istringstream stream("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E");
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  char buffer[128];
  stream.read(buffer, 2);

  EXPECT_EQ(2, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\xC3\xA4", buffer, 2));

  stream.read(buffer, 6);

  EXPECT_EQ(6, stream.gcount());
  EXPECT_EQ(0, std::memcmp(" \xE2\x82\xAC \xF0", buffer, 6));

  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(3, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\x9D\x84\x9E", buffer, 3));
}

TEST(Utf8, multiByteGet) {
  std::string content = "\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E";
  std::istringstream stream(content);
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  for (auto c : content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf16LE, simple) {
  std::istringstream stream(
      std::string("H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d\0", 22));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16LE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

TEST(Utf16LE, get) {
  std::string utf8Content = "Hello World";
  std::istringstream stream(
      std::string("H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d\0", 22));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16LE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf16LE, multiByte) {
  std::istringstream stream(
      std::string("\xE4\0 \0\xAC\x20 \0\x34\xD8\x1E\xDD", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16LE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0,
            std::memcmp("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E", buffer, 11));
}

TEST(Utf16LE, multiByteParts) {
  std::istringstream stream(
      std::string("\xE4\0 \0\xAC\x20 \0\x34\xD8\x1E\xDD", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16LE);

  char buffer[128];
  stream.read(buffer, 2);

  EXPECT_EQ(2, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\xC3\xA4", buffer, 2));

  stream.read(buffer, 6);

  EXPECT_EQ(6, stream.gcount());
  EXPECT_EQ(0, std::memcmp(" \xE2\x82\xAC \xF0", buffer, 6));

  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(3, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\x9D\x84\x9E", buffer, 3));
}

TEST(Utf16LE, multiByteGet) {
  std::string utf8Content = "\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E";
  std::istringstream stream(
      std::string("\xE4\0 \0\xAC\x20 \0\x34\xD8\x1E\xDD", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16LE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf16BE, simple) {
  std::istringstream stream(
      std::string("\0H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d", 22));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16BE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

TEST(Utf16LBE, get) {
  std::string utf8Content = "Hello World";
  std::istringstream stream(
      std::string("\0H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d", 22));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16BE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf16BE, multiByte) {
  std::istringstream stream(
      std::string("\0\xE4\0 \x20\xAC\0 \xD8\x34\xDD\x1E", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16BE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0,
            std::memcmp("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E", buffer, 11));
}

TEST(Utf16BE, multiByteParts) {
  std::istringstream stream(
      std::string("\0\xE4\0 \x20\xAC\0 \xD8\x34\xDD\x1E", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16BE);

  char buffer[128];
  stream.read(buffer, 2);

  EXPECT_EQ(2, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\xC3\xA4", buffer, 2));

  stream.read(buffer, 6);

  EXPECT_EQ(6, stream.gcount());
  EXPECT_EQ(0, std::memcmp(" \xE2\x82\xAC \xF0", buffer, 6));

  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(3, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\x9D\x84\x9E", buffer, 3));
}

TEST(Utf16BE, multiByteGet) {
  std::string utf8Content = "\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E";
  std::istringstream stream(
      std::string("\0\xE4\0 \x20\xAC\0 \xD8\x34\xDD\x1E", 12));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf16BE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf32LE, simple) {
  std::istringstream stream(
      std::string("H\0\0\0e\0\0\0l\0\0\0l\0\0\0o\0\0\0 "
                  "\0\0\0W\0\0\0o\0\0\0r\0\0\0l\0\0\0d\0\0\0",
                  44));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32LE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

TEST(Utf32LE, get) {
  std::string utf8Content = "Hello World";
  std::istringstream stream(
      std::string("H\0\0\0e\0\0\0l\0\0\0l\0\0\0o\0\0\0 "
                  "\0\0\0W\0\0\0o\0\0\0r\0\0\0l\0\0\0d\0\0\0",
                  44));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32LE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf32LE, multiByte) {
  std::istringstream stream(
      std::string("\xE4\0\0\0 \0\0\0\xAC\x20\0\0 \0\0\0\x1E\xD1\x01\0", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32LE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0,
            std::memcmp("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E", buffer, 11));
}

TEST(Utf32LE, multiByteParts) {
  std::istringstream stream(
      std::string("\xE4\0\0\0 \0\0\0\xAC\x20\0\0 \0\0\0\x1E\xD1\x01\0", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32LE);

  char buffer[128];
  stream.read(buffer, 2);

  EXPECT_EQ(2, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\xC3\xA4", buffer, 2));

  stream.read(buffer, 6);

  EXPECT_EQ(6, stream.gcount());
  EXPECT_EQ(0, std::memcmp(" \xE2\x82\xAC \xF0", buffer, 6));

  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(3, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\x9D\x84\x9E", buffer, 3));
}

TEST(Utf32LE, multiByteGet) {
  std::string utf8Content = "\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E";
  std::istringstream stream(
      std::string("\xE4\0\0\0 \0\0\0\xAC\x20\0\0 \0\0\0\x1E\xD1\x01\0", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32LE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf32BE, simple) {
  std::istringstream stream(
      std::string("\0\0\0H\0\0\0e\0\0\0l\0\0\0l\0\0\0o\0\0\0 "
                  "\0\0\0W\0\0\0o\0\0\0r\0\0\0l\0\0\0d",
                  44));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32BE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

TEST(Utf32BE, get) {
  std::string utf8Content = "Hello World";
  std::istringstream stream(
      std::string("\0\0\0H\0\0\0e\0\0\0l\0\0\0l\0\0\0o\0\0\0 "
                  "\0\0\0W\0\0\0o\0\0\0r\0\0\0l\0\0\0d",
                  44));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32BE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

TEST(Utf32BE, multiByte) {
  std::istringstream stream(
      std::string("\0\0\0\xE4\0\0\0 \0\0\x20\xAC\0\0\0 \0\x01\xD1\x1E", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32BE);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0,
            std::memcmp("\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E", buffer, 11));
}

TEST(Utf32BE, multiByteParts) {
  std::istringstream stream(
      std::string("\0\0\0\xE4\0\0\0 \0\0\x20\xAC\0\0\0 \0\x01\xD1\x1E", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32BE);

  char buffer[128];
  stream.read(buffer, 2);

  EXPECT_EQ(2, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\xC3\xA4", buffer, 2));

  stream.read(buffer, 6);

  EXPECT_EQ(6, stream.gcount());
  EXPECT_EQ(0, std::memcmp(" \xE2\x82\xAC \xF0", buffer, 6));

  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(3, stream.gcount());
  EXPECT_EQ(0, std::memcmp("\x9D\x84\x9E", buffer, 3));
}

TEST(Utf32BE, multiByteGet) {
  std::string utf8Content = "\xC3\xA4 \xE2\x82\xAC \xF0\x9D\x84\x9E";
  std::istringstream stream(
      std::string("\0\0\0\xE4\0\0\0 \0\0\x20\xAC\0\0\0 \0\x01\xD1\x1E", 20));
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf32BE);

  for (auto c : utf8Content) {
    EXPECT_EQ(static_cast<uint8_t>(c), stream.get());
  }
  EXPECT_EQ(std::char_traits<char>::eof(), stream.get());
}

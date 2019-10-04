#include <cstring>
#include <gtest/gtest.h>
#include <utf8streams.hpp>

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

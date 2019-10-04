#include <cstring>
#include <gtest/gtest.h>
#include <utf8streams.hpp>

TEST(Utf8, ascii) {
  std::istringstream stream("Hello World");
  utf8streams::UTF8StreamBuf streamBuf(stream, utf8streams::Encoding::Utf8);

  char buffer[128];
  stream.read(buffer, sizeof(buffer));

  EXPECT_EQ(11, stream.gcount());
  EXPECT_EQ(0, std::memcmp("Hello World", buffer, 11));
}

#include <fstream>
#include <iostream>
#include <utf8streams.hpp>

int main(int argc, char const *const *argv) {
  if (argc != 2) {
    std::cout << "Error: Please pass a path to a file" << std::endl;
    return 1;
  }

  std::string path(argv[1]);
  std::ifstream stream(path);

  auto encoding = utf8streams::guessEncoding(stream);
  if (encoding == utf8streams::Encoding::Unknown) {
    encoding = utf8streams::Encoding::Utf8;
  }

  utf8streams::UTF8StreamBuf streamBuf(stream, encoding);

  char buffer[4096];
  while (true) {
    stream.read(buffer, sizeof(buffer));
    auto readBytes = stream.gcount();
    if (readBytes == 0) {
      break;
    }

    std::cout << std::string(buffer, readBytes);
  }

  return 0;
}

#include <iostream>
#include <memory>
#include <print>

#include "FileSystem.hh"
#include "PhysicalStorage.hh"
#include "constants.hh"

int main(const int argc, const char* argv[]) {
  using Storage = PhysicalStorage<BLOCKS_NUMBER, BLOCK_SIZE>;

  try {
    FileSystem filesystem{std::make_unique<Storage>()};
    filesystem.create("file.txt");
    const size_t fd = filesystem.open("file.txt");
    filesystem.write(fd, "Hello world!");
    filesystem.seek(fd, 0);
    const std::string result = filesystem.read(fd, 12);
    std::print("Result: {}\n", result);
    filesystem.stat("file.txt", std::cout);
    filesystem.close(fd);
  } catch (const std::exception& exception) {
    std::print(std::cerr, "Error occured: {}\n", exception.what());
  }

  return 0;
}

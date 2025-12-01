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
    filesystem.unlink("file.txt");
    filesystem.close(fd);
  } catch (const std::exception& exception) {
    std::print(std::cerr, "Error occured: {}\n", exception.what());
  }

  return 0;
}

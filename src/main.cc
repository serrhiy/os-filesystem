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
    filesystem.ls(std::cout);
    filesystem.stat("file.txt", std::cout);
  } catch (const std::exception& exception) {
    std::print(std::cerr, "Error occured: {}\n", exception.what());
  }

  return 0;
}

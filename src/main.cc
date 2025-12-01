#include <print>

#include "PhysicalStorage.hh"
#include "constants.hh"

int main(const int argc, const char* argv[]) {
  PhysicalStorage<BLOCKS_NUMBER, BLOCK_SIZE> storage;
  std::print("hasFreeBlock: {}\n", storage.hasFreeBlock());
  const auto [span, index] = storage.getBlock();
  std::print("Block index: {}\n", index);
  return 0;
}

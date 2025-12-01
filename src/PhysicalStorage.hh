#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <utility>

#include "IStorage.hh"

template <unsigned blocksNumber, unsigned blockSize>
class PhysicalStorage final : public IStorage {
  std::array<std::array<byte_t, blockSize>, blocksNumber> blocks;
  std::array<bool, blocksNumber> isBusy;
  size_t freeBlocksNumber;
  size_t currentIndex;

  void advanceIndex() {
    for (size_t iterations = 0; iterations < blocksNumber; iterations++) {
      if (!isBusy[currentIndex]) return;
      currentIndex = (currentIndex + 1) % blocksNumber;
    }
  }

 public:
  PhysicalStorage() : currentIndex{0}, freeBlocksNumber{blocksNumber} {}

  bool hasFreeBlock() const override { return freeBlocksNumber != 0; }

  std::pair<std::span<byte_t>, size_t> getBlock() override {
    if (!hasFreeBlock()) return std::make_pair(std::span<byte_t>{}, 0);
    advanceIndex();
    freeBlocksNumber--;
    isBusy[currentIndex] = true;
    return std::make_pair(std::span<byte_t>{blocks[currentIndex]},
                          currentIndex);
  }

  void release(size_t index) override {
    if (isBusy[index]) return;
    freeBlocksNumber++;
    isBusy[index] = false;
  }
};

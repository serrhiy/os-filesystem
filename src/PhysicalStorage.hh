#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <utility>
#include <vector>

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

  bool hasFreeBlocks(size_t nblocks) const override {
    return freeBlocksNumber >= nblocks;
  }

  std::vector<block_t> getBlocks(size_t nblocks) override {
    if (!hasFreeBlocks(nblocks)) return std::vector<block_t>{};
    std::vector<block_t> result{nblocks};
    for (size_t index = 0; index < nblocks; index++) {
      advanceIndex();
      freeBlocksNumber--;
      isBusy[currentIndex] = true;
      result[index] =
          std::make_pair(std::span<byte_t>{blocks[currentIndex]}, currentIndex);
    }
    return result;
  }

  void release(std::vector<size_t> indices) override {
    for (size_t index : indices) release(index);
  }

  void release(size_t index) override {
    if (isBusy[index]) return;
    freeBlocksNumber++;
    isBusy[index] = false;
  }
};

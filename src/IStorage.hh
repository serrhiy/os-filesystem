#pragma once

#include <span>
#include <utility>
#include <vector>

class IStorage {
 public:
  using byte_t = char;
  using block_t = std::pair<std::span<byte_t>, size_t>;

  virtual bool hasFreeBlocks(size_t blocks_number = 1) const = 0;
  virtual std::vector<block_t> getBlocks(size_t blocks_number = 1) = 0;
  virtual void release(std::vector<size_t> indices) = 0;
  virtual void release(size_t index) = 0;
  virtual ~IStorage() {};
};

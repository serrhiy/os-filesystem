#pragma once

#include <span>
#include <utility>

class IStorage {
 public:
  using byte_t = unsigned char;

  virtual bool hasFreeBlock() const = 0;
  virtual std::pair<std::span<byte_t>, size_t> getBlock() = 0;
  virtual void release(size_t index) = 0;
  virtual ~IStorage() {};
};

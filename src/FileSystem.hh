#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "FileInfo.hh"
#include "IStorage.hh"

class FileSystem {
  std::unique_ptr<IStorage> storage;
  std::unordered_map<std::string, std::shared_ptr<INodeInfo>> directoryEntries;
  size_t inode_counter;

 public:
  FileSystem() = delete;
  FileSystem(std::unique_ptr<IStorage> storage);

  size_t create(const std::string filename);
  void ls(std::ostream &outputStream) const;
  void stat(const std::string& filename, std::ostream& outputStream) const;
};

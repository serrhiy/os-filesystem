#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "FileInfo.hh"
#include "IStorage.hh"

class FileSystem {
  std::unique_ptr<IStorage> storage;
  std::unordered_map<std::string, std::shared_ptr<INodeInfo>> directoryEntries;
  std::unordered_map<size_t, OpenedFileInfo> openedFiles;
  std::unordered_map<std::string, size_t> filenameToFd;
  size_t inodeCounter;
  size_t fdCounter;

  void throwIfExists(const std::string& filename) const;
  void throwIfNotExists(const std::string& filename) const;

 public:
  FileSystem() = delete;
  FileSystem(std::unique_ptr<IStorage> storage);

  size_t create(const std::string& filename);
  void ls(std::ostream& outputStream) const;
  void stat(const std::string& filename, std::ostream& outputStream) const;
  size_t link(const std::string& file1, const std::string& file2);
  void unlink(const std::string& filename);

  size_t open(const std::string filename);
};

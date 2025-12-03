#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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
  void throwIfNotOpened(size_t fd);

  size_t getRealBlocksNumber(const std::string& filename) const;

 public:
  FileSystem() = delete;
  FileSystem(std::unique_ptr<IStorage> storage);

  size_t create(const std::string& filename);
  void ls(std::ostream& outputStream) const;
  void stat(const std::string& filename, std::ostream& outputStream) const;
  size_t link(const std::string& file1, const std::string& file2);
  void unlink(const std::string& filename);

  size_t open(const std::string filename);
  void close(size_t fd);
  void seek(size_t fd, size_t offset);
  void write(size_t fd, std::string_view content);
  std::string read(size_t fd, size_t bytes);

  void truncate(const std::string& filename, size_t newSize);
};

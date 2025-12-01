#include "FileSystem.hh"

#include <format>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

#include "FileInfo.hh"

FileSystem::FileSystem(std::unique_ptr<IStorage> storage)
    : storage{std::move(storage)}, inode_counter{0} {}

size_t FileSystem::create(const std::string filename) {
  if (directoryEntries.contains(filename)) {
    static constexpr const char* message = "Failed. File {} already exists.";
    throw std::runtime_error{std::format(message, filename)};
  }
  auto fileInfo = std::make_shared<INodeInfo>(
      INodeInfo{inode_counter, FileType::REGULAR, 1, 0, 0, 0});
  directoryEntries[filename] = fileInfo;
  return inode_counter++;
}

void FileSystem::ls(std::ostream &outputStream) const {
  outputStream << "total " << directoryEntries.size() << '\n';
  for (const auto &[filename, inodeInfo] : directoryEntries) {
    outputStream << filename << '\t' << inodeInfo->nlink << '\t';
    outputStream << inodeInfo->size << '\t' << inodeInfo->inode << '\n';
  }
}

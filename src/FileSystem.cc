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

void FileSystem::ls(std::ostream& outputStream) const {
  outputStream << "total " << directoryEntries.size() << '\n';
  for (const auto &[filename, inodeInfo] : directoryEntries) {
    const char filetype = fileTypeToChar.at(inodeInfo->mode);
    outputStream << filename << '\t' << filetype << '\t';
    outputStream << inodeInfo->nlink << '\t' << inodeInfo->size << '\t';
    outputStream << inodeInfo->inode << '\n';
  }
}

void FileSystem::stat(const std::string& filename,
                      std::ostream& outputStream) const {
  if (!directoryEntries.contains(filename)) {
    static constexpr const char* message = "Failed. File {} does not exist.";
    throw std::runtime_error{std::format(message, filename)};
  }
  auto inodeInfo = directoryEntries.at(filename);
  outputStream << "File: " << filename << '\n';
  outputStream << "Size: " << inodeInfo->size << '\t';
  outputStream << "Blocks: " << inodeInfo->blocks << '\t';
  outputStream << fileTypeToDescription.at(inodeInfo->mode) << '\n';

  outputStream << "Inode: " << inodeInfo->inode << '\t';
  outputStream << "Links: " << inodeInfo->nlink << '\n';
}

size_t FileSystem::link(const std::string file1, const std::string file2) {
  if (!directoryEntries.contains(file1)) {
    static constexpr const char* message = "Failed. File {} does not exist.";
    throw std::runtime_error{std::format(message, file1)};
  }
  if (directoryEntries.contains(file2)) {
    static constexpr const char* message = "Failed. File {} already exists.";
    throw std::runtime_error{std::format(message, file2)};
  }

  auto fileInfo = directoryEntries.at(file1);
  fileInfo->nlink++;
  directoryEntries[file2] = fileInfo;
  return fileInfo->inode;
}

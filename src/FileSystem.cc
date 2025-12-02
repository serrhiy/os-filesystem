#include "FileSystem.hh"

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>

#include "FileInfo.hh"
#include "constants.hh"

using ranges_t = std::set<std::pair<size_t, size_t>>;

ranges_t::iterator getAppropriateIterator(const ranges_t& ranges, size_t start, size_t end) {
  static const auto callback = [start, end](const auto range) {
    return range.first >= start && range.first < end;
  };
  return std::find_if(ranges.begin(), ranges.end(), callback);
}

void FileSystem::throwIfExists(const std::string& filename) const {
  if (directoryEntries.contains(filename)) {
    static constexpr const char* message = "Failed. File {} already exists.";
    throw std::runtime_error{std::format(message, filename)};
  }
}

void FileSystem::throwIfNotExists(const std::string& filename) const {
  if (!directoryEntries.contains(filename)) {
    static constexpr const char* message = "Failed. File {} does not exist.";
    throw std::runtime_error{std::format(message, filename)};
  }
}

void FileSystem::throwIfNotOpened(size_t fd) {
  if (!openedFiles.contains(fd)) {
    throw std::runtime_error{std::format("Invalid fd: {}", fd)};
  }
}

size_t FileSystem::getRealFileSize(const std::string& filename) const {
  static const auto binaryOperator = [](size_t total, auto range) {
    return total + range.second - range.first;
  };
  const auto& ranges = lazyAllocation.at(filename);
  const size_t zeros =
      std::accumulate(ranges.begin(), ranges.end(), 0, binaryOperator);
  return zeros + directoryEntries.at(filename)->size;
}

FileSystem::FileSystem(std::unique_ptr<IStorage> storage)
    : storage{std::move(storage)}, inodeCounter{0}, fdCounter{0} {}

size_t FileSystem::create(const std::string& filename) {
  throwIfExists(filename);

  auto fileInfo = std::make_shared<INodeInfo>(
      INodeInfo{inodeCounter, FileType::REGULAR, 1, 0});
  directoryEntries[filename] = std::move(fileInfo);
  lazyAllocation[filename] = {};
  return inodeCounter++;
}

void FileSystem::ls(std::ostream& outputStream) const {
  outputStream << "total " << directoryEntries.size() << '\n';
  for (const auto &[filename, inodeInfo] : directoryEntries) {
    const char filetype = fileTypeToChar.at(inodeInfo->mode);
    outputStream << filename << '\t' << filetype << '\t';
    outputStream << inodeInfo->nlink << '\t' << getRealFileSize(filename)
                 << '\t';
    outputStream << inodeInfo->inode << '\n';
  }
}

void FileSystem::stat(const std::string& filename,
                      std::ostream& outputStream) const {
  throwIfNotExists(filename);

  auto inodeInfo = directoryEntries.at(filename);
  outputStream << "File: " << filename << '\n';
  outputStream << "Size: " << getRealFileSize(filename) << '\t';
  outputStream << "Blocks: " << inodeInfo->blocks.size() << '\t';
  outputStream << fileTypeToDescription.at(inodeInfo->mode) << '\n';

  outputStream << "Inode: " << inodeInfo->inode << '\t';
  outputStream << "Links: " << inodeInfo->nlink << '\n';
}

size_t FileSystem::link(const std::string& file1, const std::string& file2) {
  throwIfNotExists(file1);
  throwIfExists(file2);

  auto fileInfo = directoryEntries.at(file1);
  fileInfo->nlink++;
  directoryEntries[file2] = fileInfo;
  return fileInfo->inode;
}

void FileSystem::unlink(const std::string& filename) {
  throwIfNotExists(filename);

  auto fileInfo = directoryEntries.at(filename);
  fileInfo->nlink--;
  // If file is not opened
  if (fileInfo->nlink == 0 && !filenameToFd.contains(filename)) {
    const auto indices = fileInfo->blocks | std::views::values;
    storage->release(std::vector(indices.begin(), indices.end()));
  }

  directoryEntries.erase(filename);
}

size_t FileSystem::open(const std::string filename) {
  throwIfNotExists(filename);

  openedFiles[fdCounter] =
      OpenedFileInfo{0, filename, directoryEntries.at(filename)};
  filenameToFd[filename] = fdCounter;
  return fdCounter++;
}

void FileSystem::close(size_t fd) {
  throwIfNotOpened(fd);

  const OpenedFileInfo& openedFileinfo = openedFiles.at(fd);
  // If file was deleted while writing/reading
  if (!directoryEntries.contains(openedFileinfo.filename)) {
    const auto indices = openedFileinfo.inodeInfo->blocks | std::views::values;
    storage->release(std::vector(indices.begin(), indices.end()));
  }

  openedFiles.erase(fd);
  filenameToFd.erase(openedFileinfo.filename);
}

void FileSystem::seek(size_t fd, size_t offset) {
  throwIfNotOpened(fd);
  OpenedFileInfo& openedFileinfo = openedFiles.at(fd);
  const size_t blocksNumber = openedFileinfo.inodeInfo->blocks.size();
  if (offset < getRealFileSize(openedFileinfo.filename)) {
    openedFileinfo.position = offset;
  }
}

void FileSystem::write(size_t fd, std::string_view content) {
  throwIfNotOpened(fd);
  OpenedFileInfo& openedFileinfo = openedFiles.at(fd);

  const size_t blocksNumber = openedFileinfo.inodeInfo->blocks.size();
  const size_t writeSize = blocksNumber * BLOCK_SIZE - openedFileinfo.position;
  if (writeSize < content.size()) {
    const size_t blocksToAllocate =
        std::ceil((double)content.size() / BLOCK_SIZE);
    auto allocatedBlocks = storage->getBlocks(blocksToAllocate);
    openedFileinfo.inodeInfo->blocks.insert(
        openedFileinfo.inodeInfo->blocks.end(), allocatedBlocks.begin(),
        allocatedBlocks.end());
  }

  size_t writed = 0;
  while (writed < content.size()) {
    const size_t blockn =
        std::floor((double)openedFileinfo.position / BLOCK_SIZE);
    const size_t blockshift = openedFileinfo.position - blockn * BLOCK_SIZE;
    const size_t bytesNumberToWrite =
        std::min(BLOCK_SIZE - blockshift, content.size() - writed);
    IStorage::block_t block = openedFileinfo.inodeInfo->blocks[blockn];
    std::copy(content.begin() + writed,
              content.begin() + writed + bytesNumberToWrite,
              block.first.begin() + blockshift);
    openedFileinfo.position += bytesNumberToWrite;
    writed += bytesNumberToWrite;
  }
  if (openedFileinfo.position > openedFileinfo.inodeInfo->size) {
    openedFileinfo.inodeInfo->size = openedFileinfo.position;
  }
}

std::string FileSystem::read(size_t fd, size_t bytes) {
  throwIfNotOpened(fd);

  OpenedFileInfo& openedFileinfo = openedFiles.at(fd);

  if (openedFileinfo.position + bytes >
      getRealFileSize(openedFileinfo.filename)) {
    throw std::out_of_range{"Out of file size"};
  }

  size_t readed = 0;
  std::string result(bytes, '\0');

  const auto ranges = lazyAllocation.at(openedFileinfo.filename);
  auto range = getAppropriateIterator(ranges,
      openedFileinfo.position, openedFileinfo.position + bytes);

  while (readed < bytes) {
    if (range != ranges.end()) {
      const auto [zeroStart, zeroEnd] = *range;
      if (openedFileinfo.position >= zeroStart && openedFileinfo.position < zeroEnd) {
        readed += zeroEnd - zeroStart;
        openedFileinfo.position += zeroEnd - zeroStart;
        range++;
        continue;
      }
    }
    const size_t blockn =
        std::floor((double)openedFileinfo.position / BLOCK_SIZE);
    const size_t blockshift = openedFileinfo.position - blockn * BLOCK_SIZE;
    const size_t bytesNumberToRead =
        std::min(BLOCK_SIZE - blockshift, bytes - readed);
    auto blockBegin = openedFileinfo.inodeInfo->blocks[blockn].first.begin();
    std::copy(blockBegin + blockshift,
              blockBegin + blockshift + bytesNumberToRead,
              result.begin() + readed);
    readed += bytesNumberToRead;
    openedFileinfo.position += bytesNumberToRead;
  }
  return result;
}

void FileSystem::truncate(size_t fd, size_t size) {
  throwIfNotOpened(fd);

  OpenedFileInfo& openedFileinfo = openedFiles.at(fd);
  const size_t realSize = getRealFileSize(openedFileinfo.filename);
  const size_t blocksNumber = openedFileinfo.inodeInfo->blocks.size();

  if (realSize == size) return;

  if (realSize > size) {
    if (realSize - size >= openedFileinfo.inodeInfo->size % BLOCK_SIZE) {
      const size_t blocksNumberToRemove = std::ceil((double)size / BLOCK_SIZE);
      for (size_t i = 0; i < blocksNumberToRemove; i++) {
        storage->release(blocksNumber - i - 1);
        openedFileinfo.inodeInfo->blocks.pop_back();
      }
    }
    openedFileinfo.inodeInfo->size = size;
    if (openedFileinfo.position >= size) {
      openedFileinfo.position = size == 0 ? 0 : size - 1;
    }
    return;
  }

  lazyAllocation[openedFileinfo.filename].insert({ openedFileinfo.inodeInfo->size, size });
}

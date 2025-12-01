#pragma once

#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "IStorage.hh"

enum class FileType {
  REGULAR,
  DIRECTORY,
  SYMBOLIC,
  BLOCK,
  CHARACTER,
  PIPE,
  SOCKET
};

const std::unordered_map<FileType, char> fileTypeToChar{
    {FileType::REGULAR, '-'},   {FileType::DIRECTORY, 'd'},
    {FileType::SYMBOLIC, 'l'},  {FileType::BLOCK, 'b'},
    {FileType::CHARACTER, 'c'}, {FileType::PIPE, 'p'},
    {FileType::SOCKET, 's'},
};

const std::unordered_map<FileType, std::string> fileTypeToDescription{
    {FileType::REGULAR, "reguar file"},
    {FileType::DIRECTORY, "directory"},
    {FileType::SYMBOLIC, "symbolink link"},
    {FileType::BLOCK, "block device"},
    {FileType::CHARACTER, "character device"},
    {FileType::PIPE, "named pipe"},
    {FileType::SOCKET, "socket"},
};

struct INodeInfo {
  size_t inode;
  FileType mode;
  size_t nlink;
  size_t size;
  std::vector<IStorage::block_t> blocks;
};

struct OpenedFileInfo {
  size_t position;
  std::string filename;
  std::shared_ptr<INodeInfo> inodeInfo;
};

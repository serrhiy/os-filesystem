#pragma once

#include <unordered_map>

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

struct INodeInfo {
  size_t inode;
  FileType mode;
  size_t nlink;
  size_t size;
  size_t blksize;
  size_t blocks;
};
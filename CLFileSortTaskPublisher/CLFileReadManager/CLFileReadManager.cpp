#include "CLFileReadManager.h"
#include <fstream>
#include <algorithm>
#include <iostream>


CLFileReadManager::CLFileReadManager() {}

CLFileReadManager::~CLFileReadManager() {}

std::vector<char> CLFileReadManager::CLReadByOffset(const std::vector<std::filesystem::path>& filePaths, size_t offset, size_t length) const 
{
    std::vector<char> result;
    size_t accumulated_size = 0;
    size_t remaining_length = length;

    for (const auto& file : filePaths) 
    {
        size_t file_size = std::filesystem::file_size(file);

        
        if (offset >= accumulated_size + file_size) 
        {
            accumulated_size += file_size;
            continue;
        }

        size_t local_offset = offset > accumulated_size ? offset - accumulated_size : 0;
        size_t to_read = std::min(file_size - local_offset, remaining_length);

        CLReadFromFile(file, local_offset, to_read, result);

        remaining_length -= to_read;
        accumulated_size += file_size;

        if (remaining_length == 0) 
        {
            break;
        }
    }

    return std::move(result);
}

void CLFileReadManager::CLReadFromFile(const std::filesystem::path& file, size_t offset, size_t length, std::vector<char>& result) 
{
    std::ifstream infile(file, std::ios::binary);
    if (!infile) 
    {
        throw std::runtime_error("Failed to open file: " + file.string());
    }

    infile.seekg(offset, std::ios::beg);
    size_t to_read = length;
    result.reserve(result.size() + to_read);

    std::istreambuf_iterator<char> it(infile);
    for (int i = 0; i < to_read && it != std::istreambuf_iterator<char>(); ++i, ++it) 
    {
        result.push_back(*it);
    }
}

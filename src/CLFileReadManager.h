#ifndef CLFileReadManager_H
#define CLFileReadManager_H

#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <mutex>  

class CLFileReadManager 
{
public:
    CLFileReadManager();
    virtual ~CLFileReadManager();
    std::vector<char> CLReadByOffset(const std::vector<std::filesystem::path>& filePaths, size_t offset, size_t length) const;

private:
    static void CLReadFromFile(const std::filesystem::path& file, size_t offset, size_t length, std::vector<char>& result);
};

#endif 

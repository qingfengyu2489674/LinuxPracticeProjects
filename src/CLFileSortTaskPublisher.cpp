#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <vector>
#include <sys/stat.h>
#include <cstring>

#include "CLFileSortTaskPublisher.h"
#include "./CLFileReadManager/CLFileReadManager.h"
#include "./CLFileDataSorter/CLDataSorter.h"
#include "./CLSortedFileMerger/CLSortedFileMerger.h"


void print_int64_data(const std::vector<int64_t>& data) 
{
    std::cout << "int64_data contains:" << std::endl;
    for (const auto& value : data) 
    {
        std::cout << value << std::endl;
    }
}

void print_merged_file(const std::string& output_file) 
{
    std::ifstream infile(output_file, std::ios::binary);
    if (!infile) 
    {
        std::cerr << "Error opening output file: " << output_file << std::endl;
        return;
    }

    int64_t value;
    while (infile.read(reinterpret_cast<char*>(&value), sizeof(value))) 
    {
        std::cout << value << std::endl;
    }

    if (infile.eof()) 
    {
        std::cout << "File read complete." << std::endl;
    } 
    else 
    {
        std::cerr << "Error reading file." << std::endl;
    }
}


CLFileSortTaskPublisher::CLFileSortTaskPublisher(const std::string& dataDir, const std::string& sortedDir)
    : m_dataDir(dataDir), m_sortedDir(sortedDir)
{
    m_totalDataSize = 0;

    if (std::filesystem::exists(m_dataDir) && std::filesystem::is_directory(m_dataDir)) 
    {
        for (const auto& entry : std::filesystem::directory_iterator(m_dataDir)) 
        {
            if (std::filesystem::is_regular_file(entry)) 
            {
                m_datafilePaths.push_back(entry.path());
                m_totalDataSize += std::filesystem::file_size(entry);
            }
        }
        if (m_datafilePaths.empty()) 
        {
            std::cerr << "No files found in directory: " << m_dataDir << std::endl;
        }
    } 
    else 
    {
        std::cerr << "DataDir does not exist: " << m_dataDir << std::endl;
    }
}

CLFileSortTaskPublisher::~CLFileSortTaskPublisher() {}

void CLFileSortTaskPublisher::SetTaskPublisher(CLDataSorter* pCLDataSorter, CLSortedFileMerger* pCLSortedFileMerger, CLFileReadManager* pCLFileReadManager) 
{
    m_pCLDataSorter = pCLDataSorter;
    m_pCLSortedFileMerger = pCLSortedFileMerger;
    m_pCLFileReadManager = pCLFileReadManager;
}

std::string CLFileSortTaskPublisher::DataSortTask(size_t offset, size_t length)
{
    std::vector<char> readData = m_pCLFileReadManager->CLReadByOffset(m_datafilePaths, offset, length);
    if (reinterpret_cast<uintptr_t>(readData.data()) % alignof(int64_t) != 0) 
    {
        std::cerr << "Data is not aligned properly for 64-bit integers!" << std::endl;
        return nullptr;
    }

    size_t num_int64s = readData.size() / sizeof(int64_t);
    std::vector<int64_t> int64_data(reinterpret_cast<int64_t*>(readData.data()), reinterpret_cast<int64_t*>(readData.data()) + num_int64s);    
    m_pCLDataSorter->SortData(int64_data);

    std::string writeDir = GenerateEmptyFile();
    std::ofstream outputStream = OpenOutputStream(writeDir);
    WriteDataToFile(outputStream, int64_data);

    return std::move(writeDir);
}

std::string CLFileSortTaskPublisher::FileMergeTask(const std::vector<std::string>& filePathsToMerge)
{
    std::vector<std::ifstream> inputStreams;
    for (const auto& filePath : filePathsToMerge) 
    {
        std::ifstream inputStream = OpenInputStream(filePath);
        if (!inputStream.is_open()) 
        {
            throw std::runtime_error("Failed to open file: " + filePath);
            std::cout << "This code will not  run." << std::endl;
        }

        inputStreams.push_back(std::move(inputStream)); 
    }
    std::string writeDir = GenerateEmptyFile();

    std::ofstream outputStream = OpenOutputStream(writeDir);

    if (!outputStream.is_open()) 
    {
        std::cerr << "Failed to open output stream for file: " << writeDir << std::endl;
    }
    m_pCLSortedFileMerger->MergeFiles(inputStreams, outputStream);

    return std::move(writeDir);
}

std::ifstream CLFileSortTaskPublisher::OpenInputStream(const std::string& filePath) 
{
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) != 0) 
    {
        std::cerr << "File does not exist or cannot be accessed: " << filePath << std::endl;
        throw std::runtime_error("File does not exist: " + filePath);
    }

 
    if ((buffer.st_mode & S_IRUSR) == 0) 
    {
        std::cerr << "Permission denied: " << filePath << std::endl;
        throw std::runtime_error("Permission denied: " + filePath);
    }

    std::ifstream inputStream(filePath, std::ios::in | std::ios::binary);
    
    if (!inputStream.is_open()) 
    {
        std::cerr << "Failed to open input file: " << filePath << std::endl;
        std::cerr << "System error: " << strerror(errno) << std::endl;

        if (errno == ENOENT) 
        {
            throw std::runtime_error("File does not exist: " + filePath);
        }
        else if (errno == EACCES) 
        {
            throw std::runtime_error("Permission denied: " + filePath);
        }
        else 
        {
            throw std::runtime_error("Failed to open input file due to unknown error: " + filePath);
        }
    }

    return inputStream;
}


std::ofstream CLFileSortTaskPublisher::OpenOutputStream(const std::string& filePath) 
{
    std::ofstream outputStream(filePath, std::ios::out | std::ios::binary);
    if (!outputStream.is_open()) 
    {
        throw std::runtime_error("Failed to open output file: " + filePath);
    }
    return outputStream;
}

std::string CLFileSortTaskPublisher::GenerateEmptyFile() 
{
    uint64_t fileId = a_fileCounter.fetch_add(1, std::memory_order_relaxed);

    std::ostringstream oss;
    oss << "sorted_data_" << fileId << ".bin";

    std::filesystem::path filePath = m_sortedDir / std::filesystem::path(oss.str());

    std::ofstream outputStream(filePath, std::ios::out | std::ios::binary);
    if (!outputStream.is_open()) 
    {
        throw std::runtime_error("Failed to create an empty file: " + filePath.string());
    }

    return filePath.string();
}




void CLFileSortTaskPublisher::WriteDataToFile(std::ofstream& output, const std::vector<int64_t>& data) const 
{
    if (!output.is_open()) 
    {
        throw std::runtime_error("Output file stream is not open.");
    }

    for (const auto& value : data) 
    {
        output.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
}

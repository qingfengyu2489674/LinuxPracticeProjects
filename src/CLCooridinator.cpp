#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <filesystem>

#include "CLCooridinator.h"

class ThreadPool;
class CLFileSortTaskPublisher;

CLCooridinator::CLCooridinator(size_t readDataLength, size_t mergeFileCount)
    : readDataLength(readDataLength), mergeFileCount(mergeFileCount) {}

CLCooridinator::~CLCooridinator() 
{
    if(sortedDirFilesQueue.size() > 1)
    {   
        std::vector<std::string> filePathsToMerge = CLCooridinator::PopThreadSafe(sortedDirFilesQueue.size()); 
        std::string filePath = m_pCLFileSortTaskPublisher->FileMergeTask(filePathsToMerge);
        DeleteFiles(filePathsToMerge);
    } 
}

void CLCooridinator::SetExecObjects(CLThreadPool *pCLThreadPool, CLFileSortTaskPublisher *pFileSortTaskPublisher) 
{
    m_pCLThreadPool = pCLThreadPool;
    m_pCLFileSortTaskPublisher = pFileSortTaskPublisher;
    while (!sortedDirFilesQueue.empty()) 
	{
        sortedDirFilesQueue.pop();
    }
}

void CLCooridinator::EnqueueAllTasks() 
{
    if (!m_pCLThreadPool || !m_pCLFileSortTaskPublisher) {
        std::cerr << "ThreadPool or CLFileSortTaskPublisher is not set." << std::endl;
        return;
    }

    EnqueueSortTasks();
    while (!futures.empty()) 
    {
        auto& future = futures.front();
        future.wait(); 
        future.get();
        CheckAndEnqueueMergeTask();
        futures.erase(futures.begin());
    }
}

void CLCooridinator::EnqueueSortTasks() 
{
    uint64_t totalDataSize = m_pCLFileSortTaskPublisher->m_totalDataSize;

    for (uint64_t accumulatedOffset = 0; accumulatedOffset < totalDataSize; accumulatedOffset += readDataLength) {
        auto task = [this, accumulatedOffset]() 
        {
            this->SortTask(accumulatedOffset, this->readDataLength);
        };
        futures.push_back(m_pCLThreadPool->enqueue(task));
    }
}

void CLCooridinator::CheckAndEnqueueMergeTask() 
{
    if(sortedDirFilesQueue.size() >= mergeFileCount)
    {
        auto task = [this]() 
        {
            this->MergeTask();
        };
        futures.push_back(m_pCLThreadPool->enqueue(task));
    }
}

void CLCooridinator::SortTask(size_t offset, size_t length)
{
    std::string filePath = m_pCLFileSortTaskPublisher->DataSortTask(offset, length);
    PushThreadSafe(filePath);
}

void CLCooridinator::MergeTask()
{
    std::vector<std::string> filePathsToMerge = PopThreadSafe(mergeFileCount);
    std::string filePath = m_pCLFileSortTaskPublisher->FileMergeTask(filePathsToMerge);
    DeleteFiles(filePathsToMerge);
    PushThreadSafe(filePath);
}

void CLCooridinator::DeleteFiles(const std::vector<std::string>& filePathsToMerge)
{
    for (const auto& filePath : filePathsToMerge) 
    {
        try 
        {
            if (std::filesystem::exists(filePath)) 
            {
                std::filesystem::remove(filePath);
            } 
            else 
            {
                std::cerr << "File " << filePath << " does not exist." << std::endl;
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error deleting file " << filePath << ": " << e.what() << std::endl;
        }
    }
}

void CLCooridinator::PrintQueue() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (sortedDirFilesQueue.empty()) 
    {
        std::cout << "Queue is empty." << std::endl;
        return;
    }
    
    std::cout << "Queue contents:" << std::endl;
    std::queue<std::string> tempQueue = sortedDirFilesQueue;
    
    while (!tempQueue.empty()) 
    {
        std::cout << tempQueue.front() << std::endl;
        tempQueue.pop();
    }
}


void CLCooridinator::PushThreadSafe(const std::string& filePath) 
{
    if (filePath.empty()) 
    {
        throw std::invalid_argument("File path cannot be empty.");
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        sortedDirFilesQueue.push(filePath); 
    }

    //PrintQueue();
}

/*
std::vector<std::string> CLCooridinator::PopThreadSafe(size_t count) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (sortedDirFilesQueue.empty()) 
    {
        return std::vector<std::string>();
    }
    std::vector<std::string> result;
    result.reserve(count);  

    while (count-- > 0 && !sortedDirFilesQueue.empty()) 
    {
        result.push_back(sortedDirFilesQueue.front());
        sortedDirFilesQueue.pop();
    }
    return std::move(result);
}
*/

std::vector<std::string> CLCooridinator::PopThreadSafe(size_t count) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (sortedDirFilesQueue.empty()) 
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> result;
    result.reserve(count);  

    while (count-- > 0 && !sortedDirFilesQueue.empty()) 
    {
        std::string filePath = sortedDirFilesQueue.front();
        result.push_back(filePath);
        
        sortedDirFilesQueue.pop();
    }

    return std::move(result);
}

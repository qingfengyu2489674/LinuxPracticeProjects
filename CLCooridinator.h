#ifndef CLCooridinator_H
#define CLCooridinator_H

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <filesystem>
#include <queue>
#include <string>
#include <mutex>
#include <optional>

#include "./CLFileSortTaskPublisher/CLFileSortTaskPublisher.h"
#include "./CLThreadPool/CLThreadPool.h"

class CLThreadPool;
class CLFileSortTaskPublisher;

class CLCooridinator 
{
public:
	CLCooridinator(size_t readDataLength = 1024, size_t mergeFileCount = 2);
	virtual ~CLCooridinator();

public:
	void SetExecObjects(CLThreadPool *m_pCLThreadPool, CLFileSortTaskPublisher *CLFileSortTaskPublisher);
	void EnqueueAllTasks();
	
private:
	void EnqueueSortTasks();	
	void CheckAndEnqueueMergeTask();
	void SortTask(size_t offset, size_t length);
	void MergeTask();
	void DeleteFiles(const std::vector<std::string>& filePathsToMerge);
	void PushThreadSafe(const std::string& filePath);
	std::vector<std::string> PopThreadSafe(size_t count); 

	void PrintQueue();

private:
	size_t readDataLength;
	size_t mergeFileCount;
	std::queue<std::string> sortedDirFilesQueue;
	mutable std::mutex mutex_;
	
private:
	CLThreadPool *m_pCLThreadPool;
	CLFileSortTaskPublisher *m_pCLFileSortTaskPublisher;
	std::vector<std::future<void>> futures;
};

#endif


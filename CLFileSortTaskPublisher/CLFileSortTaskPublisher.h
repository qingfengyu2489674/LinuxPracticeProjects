#ifndef CLFileSortTaskPublisher_H
#define CLFileSortTaskPublisher_H

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <filesystem>
#include <thread>
#include <atomic>
#include "./CLFileReadManager/CLFileReadManager.h"
#include "./CLFileDataSorter/CLDataSorter.h"
#include "./CLSortedFileMerger/CLSortedFileMerger.h"


class CLDataSorter;
class CLSortedFileMerger;
class CLFileReadManager;

class CLFileSortTaskPublisher 
{
public:
	CLFileSortTaskPublisher(const std::string& dataDir, const std::string& sortedDir);
	virtual ~CLFileSortTaskPublisher();

public:
	void SetTaskPublisher(CLDataSorter *pCLDataSorter, CLSortedFileMerger *pCLSortedFileMerger, CLFileReadManager *pCLFileReadManager);
	std::string DataSortTask(size_t offset, size_t length);
	std::string FileMergeTask(const std::vector<std::string>& filePathsToMerge);

private:
	std::ifstream OpenInputStream(const std::string& filePath);
	std::ofstream OpenOutputStream(const std::string& filePath);
	std::string GenerateEmptyFile();
	void WriteDataToFile(std::ofstream& output, const std::vector<int64_t>& data) const;

public:
	std::string m_dataDir;
	std::string m_sortedDir;
	uint64_t m_totalDataSize;

private:
	std::vector<std::filesystem::path> m_datafilePaths;
	std::atomic<uint64_t> a_fileCounter{0};

private:
    CLFileSortTaskPublisher() = default;
    CLFileSortTaskPublisher(const CLFileSortTaskPublisher&) = delete;
    CLFileSortTaskPublisher& operator=(const CLFileSortTaskPublisher&) = delete;
	
private:
	CLDataSorter *m_pCLDataSorter;
	CLSortedFileMerger *m_pCLSortedFileMerger;
	CLFileReadManager *m_pCLFileReadManager;
};

#endif


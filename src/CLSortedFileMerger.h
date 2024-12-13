#ifndef FILE_MERGER_H
#define FILE_MERGER_H

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

class CLSortedFileMerger 
{
public:
    CLSortedFileMerger(size_t inputSegmentSize = 256, size_t outputSegmentSize = 512);
    virtual ~CLSortedFileMerger();

    void MergeFiles(std::vector<std::ifstream>& m_inputFiles, std::ofstream& m_outputDirectory);

public:
    size_t m_inputSegmentSize;
    size_t m_outputSegmentSize; 

private:
    std::vector<int64_t> ReadSegment(std::ifstream& infile);
    size_t FindMinIndex(const std::vector<std::vector<int64_t>>& segments, const std::vector<size_t>& indices);
    void WriteToFile(const std::vector<int64_t>& buffer, std::ofstream& m_outputDirectory);
};

#endif

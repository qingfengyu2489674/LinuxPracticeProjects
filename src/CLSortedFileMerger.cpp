#include "CLSortedFileMerger.h"
#include <iostream>
#include <limits>
#include <algorithm>

// 构造函数
CLSortedFileMerger::CLSortedFileMerger(size_t inputSegmentSize, size_t outputSegmentSize)
      :m_inputSegmentSize(inputSegmentSize), m_outputSegmentSize(outputSegmentSize) {}

CLSortedFileMerger::~CLSortedFileMerger(){}

void CLSortedFileMerger::MergeFiles(std::vector<std::ifstream>& inputStreams, std::ofstream& outputStream) 
{

    std::vector<std::vector<int64_t>> segments(inputStreams.size());
    for (size_t i = 0; i < inputStreams.size(); ++i) 
    {   
        segments[i] = ReadSegment(inputStreams[i]); 
    }

    std::vector<int64_t> merged;
    std::vector<size_t> indices(inputStreams.size(), 0);
    while (true) 
    {
        while (merged.size() < m_outputSegmentSize) 
        {
            size_t min_index = FindMinIndex(segments, indices);
            if (min_index == segments.size()) 
            {
                break;
            }

            merged.push_back(segments[min_index][indices[min_index]++]);

            if (indices[min_index] == segments[min_index].size()) 
            {
                segments[min_index] = ReadSegment(inputStreams[min_index]);
                indices[min_index] = 0;
            }
        }

        if (!merged.empty()) 
        {
            WriteToFile(merged, outputStream);
            merged.clear();
        }

        if (std::all_of(segments.begin(), segments.end(), [](const std::vector<int64_t>& segment) 
        {
            return segment.empty();
        })) 

        {
            break;
        }
    }
}

std::vector<int64_t> CLSortedFileMerger::ReadSegment(std::ifstream& infile) 
{
    std::vector<int64_t> data(m_inputSegmentSize);
    infile.read(reinterpret_cast<char*>(data.data()), m_inputSegmentSize * sizeof(int64_t));

    size_t bytes_read = infile.gcount();
    if (bytes_read == 0) 
    {
        return {};
    }

    size_t elements_read = bytes_read / sizeof(int64_t);
    data.resize(elements_read);
    return std::move(data);
}

size_t CLSortedFileMerger::FindMinIndex(const std::vector<std::vector<int64_t>>& segments, const std::vector<size_t>& indices) 
{
    size_t min_index = segments.size();
    int64_t min_value = std::numeric_limits<int64_t>::max();

    for (size_t i = 0; i < segments.size(); ++i) 
    {
        if (indices[i] < segments[i].size() && segments[i][indices[i]] < min_value) 
        {
            min_value = segments[i][indices[i]];
            min_index = i;
        }
    }

    return min_index;
}

void CLSortedFileMerger::WriteToFile(const std::vector<int64_t>& buffer,  std::ofstream& outputStream) 
{
    outputStream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(int64_t));
}

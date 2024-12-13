#include "CLDataSorter.h"
#include <algorithm>

void CLDataSorter::SortData(std::vector<int64_t>& data) 
{
    std::sort(data.begin(), data.end());
}

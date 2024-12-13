#include "CLCooridinator.h"

int main() 
{
    std::string dataDir = "./test_data";
    std::string sortedDir = "./sorted_files";
    
    CLDataSorter dataSorter;
    CLSortedFileMerger sortedFileMerger(4096, 4096*2);
    CLFileReadManager fileReadManager;
    CLFileSortTaskPublisher fileSortTaskPublisher(dataDir, sortedDir);
    fileSortTaskPublisher.SetTaskPublisher(&dataSorter, &sortedFileMerger, &fileReadManager);

    CLThreadPool threadPool(4);
    CLCooridinator coordinator(4096*4, 4);
    
    coordinator.SetExecObjects(&threadPool, &fileSortTaskPublisher);
    coordinator.EnqueueAllTasks();
}

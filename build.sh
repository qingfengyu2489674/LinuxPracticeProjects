g++ -std=c++17 -pthread \
    ./CLFileSortTaskPublisher/CLFileDataSorter/CLDataSorter.cpp \
    ./CLFileSortTaskPublisher/CLFileReadManager/CLFileReadManager.cpp \
    ./CLFileSortTaskPublisher/CLFileSortTaskPublisher.cpp \
    ./CLFileSortTaskPublisher/CLSortedFileMerger/CLSortedFileMerger.cpp \
    ./CLThreadPool/CLThreadPool.cpp \
    ./CLCooridinator.cpp \
    ./main.cpp \
    -g -o main

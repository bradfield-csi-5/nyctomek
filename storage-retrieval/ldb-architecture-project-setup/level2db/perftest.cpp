#include <fstream>
#include <iostream>
#include <level2db_database.h>
#include <level2db_iterator.h>
#include <chrono>
#include <cstdlib>

int main(int argc, char *argv[]) {

    bool useSkipList = false;
    if(argc > 1 && argv[1] == "--skiplist") {
        useSkipList = true;
    }

    const int MAX_LINE_LENGTH = 1024;
    char lineBuffer[MAX_LINE_LENGTH] = {0};

    std::ifstream inputFile("data/movies.csv");

    std::vector<std::pair<std::string,std::string>> idTitlePairs;
    while(inputFile.getline(lineBuffer, sizeof lineBuffer)) {
        std::string s{lineBuffer};
        size_t firstComma = s.find_first_of(',');
        size_t lastComma = s.find_last_of(',');
        std::string id = s.substr(0, firstComma);
        std::string title = s.substr(firstComma + 1, lastComma - firstComma - 1);
        idTitlePairs.emplace_back(std::make_pair(id, title));
    }

    // Randomize the data.
    std::sort(idTitlePairs.begin(), idTitlePairs.end(), [](auto &lhs, auto &rhs) { return std::rand() % 2 == 0; });

    Level2DB::Database db{useSkipList};
    auto start = std::chrono::high_resolution_clock::now();
    for(auto &kv : idTitlePairs) {
        db.Put(kv.first, kv.second);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Records: " << idTitlePairs.size() 
              << ".\nElapsed time: " << duration.count() << " microseconds" 
              << ".\nPuts per microsecond: " << static_cast<double>(idTitlePairs.size()) / duration.count() << ". "<< std::endl;
}
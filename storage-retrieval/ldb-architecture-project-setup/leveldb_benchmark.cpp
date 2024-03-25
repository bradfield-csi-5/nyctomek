#include <leveldb/db.h>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <random_data_generator.h>
#include <chrono>

using KVPairs = std::vector<std::pair<std::string, std::string>>;

void benchmarkLevelDB(const KVPairs &kvPairs, const char *dbname) {

    // Open the database.
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    std::string dbDirectory = "/tmp/";
    dbDirectory += dbname;
    leveldb::Status status = leveldb::DB::Open(options, dbDirectory, &db);
    assert(status.ok());

    // Analyze how much data we're writing.
    unsigned int numBytes = 0;
    for(const auto kvPair : kvPairs) {
        numBytes += kvPair.first.size() + kvPair.second.size();
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(const auto kvPair : kvPairs) {
        status = db->Put(leveldb::WriteOptions(), kvPair.first, kvPair.second);
        assert(status.ok());
    }
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Records: " << kvPairs.size() << ", bytes: " << numBytes << "."
              << "\nElapsed time: " << duration.count() << " microseconds."
              << "\nThroughput: " << static_cast<double>(numBytes) / duration.count() * 1000000 / 1048576 << " MB/sec.\n";

    delete db;

}

int main(int argc, char *argv[]) {

    KVPairs kvPairs = {
        { "Line 1",  "Deep Work." },
        { "Line 2",  "LevelDB Exploration." },
        { "Line 3",  "Write a simple program that reads & writes key-value data in a local LevelDB database." },
        { "Line 4",  "Document what files are created when a LevelDB is created. "},
        { "Line 5",  "Document what files are touched when key-value data is deleted from a LevelDB." },
        { "Line 6",  "Write a benchmark program that generates random key-value pairs." },
        { "Line 7",  "TomekDB." },
        { "Line 8",  "Define an interface for interacting with TomekDB." },
        { "Line 9",  "Implement the interface for interacting with TomekDB." },
        { "Line 10", "Add GoogleTests for the interface for interacting with TomekDB." },
        { "Line 11", "Define an interface for TomekDBIterator." },
        { "Line 12", "Implement the interface for TomekDBIterator." },
        { "Line 13", "Add GoogleTests for the interface for TomekDBIterator." },
        { "Line 14", "Write a benchmark program for TomekDB." }
    };

    std::cout << "Simple Write Test." << std::endl;
    benchmarkLevelDB(kvPairs, "simpletestdb");

    std::cout << "Random Write Test." << std::endl;
    kvPairs.clear();
    int numRecords = 100 * 1000, keyLength = 64, valueLength = 256;
    populateKVPairs(kvPairs, numRecords, keyLength, valueLength, DataOrdering::RANDOM);
    benchmarkLevelDB(kvPairs, "randomtestdb");

    std::cout << "Sequential Write Test." << std::endl;
    kvPairs.clear();
    populateKVPairs(kvPairs, numRecords, keyLength, valueLength, DataOrdering::SEQUENTIAL);
    benchmarkLevelDB(kvPairs, "sequentialtestdb");

    return 0;
}
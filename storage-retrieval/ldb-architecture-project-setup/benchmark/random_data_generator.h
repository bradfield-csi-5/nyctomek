#ifndef RANDOM_DATA_GENERATOR
#define RANDOM_DATA_GENERATOR

#include <vector>
#include <utility>
#include <string>

void generateRandomData(char *output, unsigned int length);

enum class DataOrdering : short {
    RANDOM,
    SEQUENTIAL
};

using KVPairs = std::vector<std::pair<std::string, std::string>>;
void populateKVPairs(KVPairs &kvPairs, size_t numRecords, size_t keyLength, size_t valueLength, DataOrdering ordering);


#endif
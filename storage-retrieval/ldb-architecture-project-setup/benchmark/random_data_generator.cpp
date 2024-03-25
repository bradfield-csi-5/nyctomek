#include <random_data_generator.h>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <utility>

void generateRandomData(char *output, unsigned int length) {

    std::srand(std::time(nullptr));
    char space = ' ';
    char tilde = '~';
    for(unsigned int index = 0; index < length-1; ++index) {
        output[index] = space + (std::rand() % (tilde - space + 1));
    }
    output[length-1] = '\0';
}

void populateKVPairs(KVPairs &kvPairs,
                     size_t numRecords,
                     size_t keyLength,
                     size_t valueLength,
                     DataOrdering ordering) {

    char *keyBuffer = (char*)malloc(keyLength);
    char *valueBuffer = (char*)malloc(valueLength);

    for(size_t count = 0; count < numRecords; ++count) {
        generateRandomData(keyBuffer, keyLength);
        generateRandomData(valueBuffer, keyLength);
        kvPairs.emplace_back(std::pair{keyBuffer, valueBuffer});
    }

    if(ordering == DataOrdering::SEQUENTIAL) {
        std::sort(kvPairs.begin(), kvPairs.end(),
            [](auto &lhs, auto &rhs) { return lhs.first < rhs.first; });
    }
    free(keyBuffer);
    free(valueBuffer);
}

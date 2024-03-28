#include <fstream>
#include <iostream>
#include <level2db_database.h>
#include <level2db_iterator.h>

int main(int argc, char *argv[]) {

    const int MAX_LINE_LENGTH = 1024;
    char lineBuffer[MAX_LINE_LENGTH] = {0};

    std::ifstream inputFile("data/movies.csv");
    Level2DB::Database db;

    while(inputFile.getline(lineBuffer, sizeof lineBuffer)) {
        std::string s{lineBuffer};
        size_t firstComma = s.find_first_of(',');
        size_t lastComma = s.find_last_of(',');
        std::string id = s.substr(0, firstComma);
        std::string title = s.substr(firstComma + 1, lastComma - firstComma - 1);
        db.Put(id, title);
    }

    auto rangeScanResult = db.RangeScan("1", "999999");
    Level2DB::Iterator *iterator = dynamic_cast<Level2DB::Iterator*>(
        std::get<Level2DB::IteratorInterface*>(rangeScanResult));
    do {
        std::cout << "{ " << iterator->Key() << ", " << iterator->Value()  << " }" << std::endl;//<< iterator->Value() << "}" << std::endl;
    } while(iterator->Next());
}
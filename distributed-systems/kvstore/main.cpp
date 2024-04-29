#include <string>
#include <csignal>
#include <fstream>
#include <iostream>
#include <optional>
#include <algorithm>
#include <unordered_map>

const char DATA_FILE[] = "datastore.dat";

enum class CommandType : int {
    GET,
    SET
};

struct Command {
    CommandType d_commandType;
    std::string d_key;
    std::string d_value;
};

std::pair<bool, std::optional<std::string>>
processCommand(const Command &command, std::unordered_map<std::string, std::string> &kvStore) {

    bool returnCode = true;
    std::optional<std::string> returnValue;

    if(command.d_commandType == CommandType::GET) {
        if(kvStore.contains(command.d_key)) {
            returnCode  = true;
            returnValue = kvStore[command.d_key];
        }
        else {
            returnCode  = false;
        }
    }

    if(command.d_commandType == CommandType::SET) {
        kvStore[command.d_key] = command.d_value;
        returnCode = true;
        std::ofstream dataFile(DATA_FILE, std::ios::out | std::ios::trunc);
        for(auto kv : kvStore) {
            dataFile << kv.first << "," << kv.second << std::endl;
        }
        dataFile.close();

    }
    return std::pair(returnCode, returnValue);
}

bool containsIllegalChars(const std::string &token) {
    std::vector<char> illegalChars{ ' ', '=', ','};
    return std::any_of(illegalChars.begin(), illegalChars.end(),
        [&token](char c) { return token.find(c) != std::string::npos; });
}

void usage() {
    std::cout << "Usage:\n\nGet value by key: get [key]\nSet value by key: set [key]=[value]\nQuit:quit()\n";
}

std::pair<bool, Command>
handleInput(const std::string &input) {
    std::string key, value;
    // Skip "get ", "set ".
    const size_t keyStart = 4;
    bool invalidInput = false;
    CommandType commandType;

    if(input == "quit()") {
        exit(0);
    }
    else if(input.starts_with("get ")) {
        commandType = CommandType::GET;
        key = input.substr(keyStart);
    }
    else if(input.starts_with("set ")) {
        commandType = CommandType::SET;
        size_t keyEnd = input.find('=');
        if(keyEnd != std::string::npos) {
            key = input.substr(keyStart, keyEnd-keyStart);
            value = input.substr(keyEnd + 1);
        }
        else {
            invalidInput = true;
        }
    }
    else {
        invalidInput = true;
    }
    if(containsIllegalChars(key) ||
       containsIllegalChars(value)) {
        invalidInput = true;
    }
    if(invalidInput) {
        std::cout << "\nInvalid input.\n\n";
        usage();
    }
    Command command;
    if(!invalidInput) {
        command.d_commandType = commandType;
        command.d_key = key;
        command.d_value = value;
    }
    return std::make_pair(!invalidInput, command);
}

void initializeFromDataStore(std::unordered_map<std::string, std::string> &kvStore) {
    const int MAX_LINE_LENGTH = 1024;
    char lineBuffer[MAX_LINE_LENGTH] = {0};
    std::ifstream dataFile(DATA_FILE);
    if(dataFile.is_open()) {
        while(dataFile.getline(lineBuffer, sizeof lineBuffer)) {
            std::string line{lineBuffer};
            size_t commaPosition = line.find(',');
            std::string key = line.substr(0, commaPosition);
            std::string value = line.substr(commaPosition + 1);
            kvStore[key] = value;
        }
    }

}

void mainLoop() {
    std::unordered_map<std::string, std::string> kvStore;
    initializeFromDataStore(kvStore);
    std::string input;
    while(1) {
        std::cout << "\n> ";
        std::getline(std::cin, input);
        auto [validInput, command] = handleInput(input);
        if(validInput) {
             auto [status, value] = processCommand(command, kvStore);
            if(CommandType::GET == command.d_commandType) {
                std::cout << "\n";
                if(status) {
                    std::cout << "'" << *value << "'";
                }
                else {
                    std::cout << "Key '" << command.d_key << "' not found.";
                }
                std::cout << "\n";
            }
        }
    }
}

void signalHandler(int signal) {
    exit(0);
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signalHandler);
    mainLoop();
    return 0;
}
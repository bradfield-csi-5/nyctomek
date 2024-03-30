#ifndef LEVEL2DB_COMMON
#define LEVEL2DB_COMMON

#include <string>

namespace Level2DB {

using Bytes = std::string;

enum class ErrorCode : short {
    KEY_NOT_FOUND,
    END_OF_RANGE,
    GENERIC_ERROR
};

}

#endif
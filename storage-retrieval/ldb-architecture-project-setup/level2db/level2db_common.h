#ifndef LEVEL2DB_COMMON
#define LEVEL2DB_COMMON

#include <string>

namespace Level2DB {

using Bytes = std::string;

enum class ErrorCode : short {
    KEY_NOT_FOUND,
    GENERIC_ERROR
};

}

#endif
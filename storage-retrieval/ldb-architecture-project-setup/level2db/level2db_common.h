#ifndef LEVEL2DB_COMMON
#define LEVEL2DB_COMMON

#include <vector>

namespace Level2DB {

using Bytes = std::vector<std::byte>;

enum class ErrorCode : short {
    KEY_NOT_FOUND,
    GENERIC_ERROR
};

}

#endif
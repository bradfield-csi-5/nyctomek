#ifndef LEVEL2DB_ITERATOR
#define LEVEL2DB_ITERATOR

#include <level2db_common.h>
#include <optional>

namespace Level2DB {

class Iterator {

bool Next();

std::optional<ErrorCode> Error();

Bytes Key();

Bytes Value();


};
}

#endif
#ifndef LEVEL2DB_INTERFACE
#define LEVEL2DB_INTERFACE

#include <level2db_common.h>
#include <cstddef>
#include <vector>
#include <variant>
#include <optional>

namespace Level2DB {

class Level2DBIterator;

class Interface {

using Bytes = std::vector<std::byte>;

std::variant<ErrorCode, Bytes> Get(const Bytes &key);

bool Has(const Bytes &key);

std::optional<ErrorCode> Put(const Bytes &key, const Bytes &value);

std::optional<ErrorCode> Delete(const Bytes &key);

std::variant<Level2DBIterator, ErrorCode> RangeScan(const Bytes &start, const Bytes &end);

};

}

#endif
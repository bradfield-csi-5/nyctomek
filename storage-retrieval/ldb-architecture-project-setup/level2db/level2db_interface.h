#ifndef LEVEL2DB_INTERFACE
#define LEVEL2DB_INTERFACE

#include <level2db_common.h>
#include <cstddef>
#include <variant>
#include <optional>

namespace Level2DB {

class IteratorInterface;

class Interface {

public:

    virtual std::variant<ErrorCode, Bytes> Get(const Bytes &key) = 0;

    virtual bool Has(const Bytes &key) = 0;

    virtual std::optional<ErrorCode> Put(const Bytes &key, const Bytes &value) = 0;

    virtual std::optional<ErrorCode> Delete(const Bytes &key) = 0;

    virtual std::variant<IteratorInterface, ErrorCode> RangeScan(const Bytes &start, const Bytes &end) = 0;

};

}

#endif
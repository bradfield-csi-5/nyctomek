#ifndef LEVEL2DB_DATABASE
#define LEVEL2DB_DATABASE

#include <level2db_interface.h>
#include <skiplist.h>
#include <map>

namespace Level2DB {

class Iterator;

class Database : public Interface {

public:

    Database(bool useSkipList=false) : m_useSkipList{useSkipList} {}
    virtual std::variant<ErrorCode, Bytes> Get(const Bytes &key);

    virtual bool Has(const Bytes &key);

    virtual std::optional<ErrorCode> Put(const Bytes &key, const Bytes &value);

    virtual std::optional<ErrorCode> Delete(const Bytes &key);

    virtual std::variant<ErrorCode, IteratorInterface*> RangeScan(const Bytes &start, const Bytes &end);

private:

    using KVStore = std::map<Bytes, Bytes>;
    using SLStore = SkipList<std::string, std::string, LevelGenerator>;

    bool m_useSkipList;
    KVStore m_keyValueStore;
    SLStore m_skipListKVStore;

friend class Iterator;

};
}

#endif
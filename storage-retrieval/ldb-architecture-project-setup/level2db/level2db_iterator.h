#ifndef LEVEL2DB_ITERATOR
#define LEVEL2DB_ITERATOR

#include <level2db_iteratorinterface.h>
#include <level2db_database.h>

namespace Level2DB {

class Iterator : public IteratorInterface {

public:
    Iterator(Database::KVStore::const_iterator &start, Database::KVStore::const_iterator &end);

    virtual bool Next();

    virtual std::optional<ErrorCode> Error();

    virtual Bytes Key();

    virtual Bytes Value();

private:
    Database::KVStore::const_iterator m_current;
    Database::KVStore::const_iterator m_end;
};
}

#endif
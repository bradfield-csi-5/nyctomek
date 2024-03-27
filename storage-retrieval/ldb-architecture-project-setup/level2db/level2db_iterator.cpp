#include <level2db_iterator.h>
#include <cassert>

namespace Level2DB {

Iterator::Iterator(
    Database::KVStore::const_iterator &start, 
    Database::KVStore::const_iterator &end) :
    m_current{start},
    m_end{end}{
}

bool Iterator::Next() {
    return m_current != m_end;
}

std::optional<ErrorCode> Iterator::Error() {
    return std::nullopt;
}

Bytes Iterator::Key() {
    assert(Next());
    return m_current->first;
}

Bytes Iterator::Value() {
    assert(Next());
    return m_current->second;
}
}
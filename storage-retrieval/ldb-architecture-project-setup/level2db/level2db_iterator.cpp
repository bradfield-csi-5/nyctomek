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
    if(m_current == m_end || ++m_current == m_end) {
        return false;
    }
    return true;
}

std::optional<ErrorCode> Iterator::Error() {
    return std::nullopt;
}

Bytes Iterator::Key() {
    return m_current->first;
}

Bytes Iterator::Value() {
    return m_current->second;
}
}
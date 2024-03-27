#include <level2db_database.h>

namespace Level2DB {

std::variant<ErrorCode, Bytes> Database::Get(const Bytes &key) {
    const auto it = m_keyValueStore.find(key);
    if(m_keyValueStore.end() == it) {
        return ErrorCode::KEY_NOT_FOUND;
    }
    return it->second;
}

bool Database::Has(const Bytes &key) {
    return m_keyValueStore.count(key);
}

std::optional<ErrorCode> Database::Put(const Bytes &key, const Bytes &value) {
    m_keyValueStore[key] = value;
    return std::nullopt;
}

std::optional<ErrorCode> Database::Delete(const Bytes &key) {
    size_t keysErased = m_keyValueStore.erase(key);
    if(0 == keysErased) {
        return ErrorCode::KEY_NOT_FOUND;
    }
    return std::nullopt;
    
}

}
#ifndef LEVEL2DB_ITERATORINTERFACE
#define LEVEL2DB_ITERATORINTERFACE

#include <level2db_common.h>
#include <optional>

namespace Level2DB {

class IteratorInterface {

virtual bool Next() = 0;

virtual std::optional<ErrorCode> Error() = 0;

virtual Bytes Key() = 0;

virtual Bytes Value() = 0;


};
}

#endif
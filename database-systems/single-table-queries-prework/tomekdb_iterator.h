#ifndef TOMEKDB_ITERATOR
#define TOMEKDB_ITERATOR

#include <tomekdb_tuple.h>

namespace tomekdb {
class Iterator {
public:
  virtual const Tuple* next() = 0;
  virtual void close() = 0;
};
}

#endif

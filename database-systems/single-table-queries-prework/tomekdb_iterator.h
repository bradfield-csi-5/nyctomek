#ifndef TOMEKDB_ITERATOR
#define TOMEKDB_ITERATOR

#include <tuple.h>

namespace tomekdb {
class iterator {
  virtual tuple next() = 0;
  virtual void close() = 0;
};
}

#endif TOMEKDB_ITERATOR

#ifndef TOMEKDB_ITERATOR
#define TOMEKDB_ITERATOR

#include <tuple.h>

namespace tomekdb {
class iterator {
  tuple next() = 0;
  void close() = 0;
};
}

#endif TOMEKDB_ITERATOR

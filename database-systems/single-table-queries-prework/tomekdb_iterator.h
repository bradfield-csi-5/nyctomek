#ifndef TOMEKDB_ITERATOR
#define TOMEKDB_ITERATOR

#include <tomekdb_tuple.h>
#include <optional>

namespace tomekdb
{
  class Iterator
  {
  public:
    virtual std::optional<Tuple> next() = 0;
    virtual void close() = 0;
  };
}

#endif

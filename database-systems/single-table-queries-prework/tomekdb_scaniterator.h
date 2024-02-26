#ifndef SCAN_ITERATOR
#define SCAN_ITERATOR

#include <tomekdb_iterator.h>

namespace tomekdb {
class ScanIterator : public Iterator {
public:
  ScanIterator(std::list<Tuple>& tuples);
  virtual const Tuple* next();
  virtual void close();
private:
    std::list<Tuple> d_tuples;
    std::list<Tuple>::const_iterator d_currentTuple;
};
}
#endif
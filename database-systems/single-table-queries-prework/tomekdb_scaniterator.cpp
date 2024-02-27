#include <tomekdb_scaniterator.h>

namespace tomekdb
{

    ScanIterator::ScanIterator(std::list<Tuple> &tuples) : d_tuples{tuples},
                                                           d_currentTuple{d_tuples.begin()}
    {
    }

    const Tuple *ScanIterator::next()
    {
        return (d_currentTuple == d_tuples.end()) ? nullptr : &(*d_currentTuple++);
    }

    void ScanIterator::close()
    {
    }
}

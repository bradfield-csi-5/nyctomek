#include <tomekdb_scaniterator.h>

namespace tomekdb
{

    ScanIterator::ScanIterator(std::list<Tuple> &tuples) : d_tuples{tuples},
                                                           d_currentTuple{d_tuples.begin()}
    {
    }

    std::optional<Tuple> ScanIterator::next()
    {
        return (d_currentTuple == d_tuples.end())
                   ? std::nullopt
                   : std::optional<Tuple>(*d_currentTuple++);
    }

    void ScanIterator::close()
    {
    }
}

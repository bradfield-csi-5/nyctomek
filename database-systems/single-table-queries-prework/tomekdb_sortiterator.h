#ifndef TOMEKDB_SORTITERATOR
#define TOMEKDB_SORTITERATOR

#include <tomekdb_iterator.h>
#include <vector>

namespace tomekdb {

class SortIterator : public Iterator {
public:

    enum class SortOrder {
        ASCENDING,
        DESCENDING
    };

    SortIterator(const std::string& sortColumnName, SortOrder sortOrder, Iterator &child);

    virtual std::optional<Tuple> next() override;
    virtual void close() override;

private:
    std::string                        d_sortColumnName;
    SortOrder                          d_sortOrder;
    Iterator                          &d_child;
    std::vector<Tuple>                 d_tuples;
    std::vector<Tuple>::const_iterator d_currentRecord;
};

}

#endif
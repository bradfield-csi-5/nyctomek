#ifndef TOMEKDB_LIMITITERATOR
#define TOMEKDB_LIMITITERATOR

#include <tomekdb_iterator.h>

namespace tomekdb
{
    class LimitIterator : public Iterator
    {
    public:
        LimitIterator(size_t limit, Iterator &child);
        virtual std::optional<Tuple> next() override;
        virtual void close() override;

    private:
        size_t d_limit;
        Iterator &d_child;
    };
}

#endif
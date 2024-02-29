#ifndef TOMEKDB_PROJECTIONITERATOR
#define TOMEKDB_PROJECTIONITERATOR

#include <list>
#include <string>
#include <tomekdb_tuple.h>
#include <tomekdb_iterator.h>

namespace tomekdb
{
    class ProjectionIterator : public Iterator
    {
    public:
        ProjectionIterator(const std::list<std::string> &fieldNames, Iterator *child);
        virtual const Tuple *next() override;
        virtual void close() override;

    private:
        std::list<std::string> d_fieldNames;
        Iterator *d_child;
    };
}
#endif
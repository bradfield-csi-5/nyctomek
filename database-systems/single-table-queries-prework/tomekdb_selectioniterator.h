#ifndef TOMEKDB_SELECTIONITERATOR
#define TOMEKDB_SELECTIONITERATOR

#include <string>
#include <variant>
#include <tomekdb_iterator.h>

namespace tomekdb
{
    class SelectionCriteria
    {
    public:
        enum class ComparisonOperator
        {
            EQUALS,
            NOT_EQUAL,
            GREATER_THAN,
            LESS_THAN
        };

        SelectionCriteria(const std::string &fieldName,
                          const std::variant<int, double, std::string> &fieldValue,
                          ComparisonOperator comparisonOperator);

        const std::string &fieldName() const;
        const std::variant<int, double, std::string> &fieldValue() const;
        ComparisonOperator comparisonOperator() const;

    private:
        std::string d_fieldName;
        std::variant<int, double, std::string> d_fieldValue;
        ComparisonOperator d_comparisonOperator;
    };

    class SelectionIterator : public Iterator
    {
    public:
        SelectionIterator(const SelectionCriteria &selectionCriteria, Iterator *child);
        virtual const Tuple *next() override;
        virtual void close() override;

    private:
        bool selectionMatches(const Tuple *tuple);
        SelectionCriteria d_selectionCriteria;
        Iterator *d_child;
    };

}

#endif
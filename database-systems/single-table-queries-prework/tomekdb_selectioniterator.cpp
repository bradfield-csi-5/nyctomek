#include <tomekdb_selectioniterator.h>

namespace tomekdb
{

    SelectionCriteria::SelectionCriteria(
        const std::string &fieldName,
        const std::variant<int, double, std::string> &fieldValue,
        SelectionCriteria::ComparisonOperator comparisonOperator)
        : d_fieldName{fieldName},
          d_fieldValue{fieldValue},
          d_comparisonOperator{comparisonOperator}
    {
    }

    const std::string &SelectionCriteria::fieldName() const
    {
        return d_fieldName;
    }

    const std::variant<int, double, std::string> &SelectionCriteria::fieldValue() const
    {
        return d_fieldValue;
    }

    SelectionCriteria::ComparisonOperator SelectionCriteria::comparisonOperator() const
    {
        return d_comparisonOperator;
    }

    SelectionIterator::SelectionIterator(
        const SelectionCriteria &selectionCriteria,
        Iterator *child)
        : d_selectionCriteria{selectionCriteria},
          d_child{child}
    {
    }

    const Tuple *SelectionIterator::next()
    {
        while (const Tuple *tuple = d_child->next())
        {
            if (selectionMatches(tuple))
            {
                return tuple;
            }
        }
        return nullptr;
    }

    bool SelectionIterator::selectionMatches(const Tuple *tuple)
    {
        for (const auto field : tuple->fields())
        {
            if (field.name() == d_selectionCriteria.fieldName())
            {
                const std::variant<int, double, std::string> &selectionValue =
                    d_selectionCriteria.fieldValue();
                switch (d_selectionCriteria.comparisonOperator())
                {
                case SelectionCriteria::ComparisonOperator::EQUALS:
                    return field.data() == selectionValue;
                case SelectionCriteria::ComparisonOperator::NOT_EQUAL:
                    return field.data() != selectionValue;
                case SelectionCriteria::ComparisonOperator::GREATER_THAN:
                    return field.data() > selectionValue;
                case SelectionCriteria::ComparisonOperator::LESS_THAN:
                    return field.data() < selectionValue;
                default:
                    return false; // TODO: throw.
                }
            }
        }
        return false;
    }

    void SelectionIterator::close()
    {
    }
}
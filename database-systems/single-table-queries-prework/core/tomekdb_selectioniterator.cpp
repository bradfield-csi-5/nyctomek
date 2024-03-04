#include <tomekdb_selectioniterator.h>
#include <sstream>

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
        Iterator &child)
        : d_selectionCriteria{selectionCriteria},
          d_child{child}
    {
    }

    std::optional<Tuple> SelectionIterator::next()
    {
        while (std::optional<Tuple> tuple{d_child.next()})
        {
            if (selectionMatches(tuple.value()))
            {
                return tuple;
            }
        }
        return std::nullopt;
    }

    bool SelectionIterator::selectionMatches(const Tuple &tuple)
    {
        for (const auto field : tuple.fields())
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

                    std::ostringstream os;
                    os << "Invalid comparison operator: "
                       << static_cast<int>(d_selectionCriteria.comparisonOperator())
                       << ".";
                    throw std::runtime_error(os.str());
                }
            }
        }
        return false;
    }

    void SelectionIterator::close()
    {
        d_child.close();
    }
}

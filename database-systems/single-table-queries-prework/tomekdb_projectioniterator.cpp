#include <tomekdb_projectioniterator.h>
#include <iterator>
#include <algorithm>

namespace tomekdb
{

    ProjectionIterator::ProjectionIterator(
        const std::list<std::string> &fieldNames,
        Iterator &child)
        : d_fieldNames{fieldNames},
          d_child{child}
    {
    }

    std::optional<Tuple> ProjectionIterator::next()
    {
        std::optional<Tuple> nextTuple = d_child.next();
        if (nextTuple)
        {
            std::list<Field> projectedFields;
            for (auto &name : d_fieldNames)
            {
                const std::list<Field> &fields = nextTuple->fields();
                std::copy_if(fields.begin(), fields.end(), std::back_inserter(projectedFields),
                             [&name](const Field &field)
                             { return field.name() == name; });
            }
            if (projectedFields.size())
            {
                return Tuple{projectedFields};
            }
        }
        return std::nullopt;
    }

    void ProjectionIterator::close()
    {
        d_child.close();
    }
}
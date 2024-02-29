#include <tomekdb_projectioniterator.h>
#include <iterator>
#include <algorithm>

namespace tomekdb
{

    ProjectionIterator::ProjectionIterator(
        const std::list<std::string> &fieldNames,
        Iterator *child)
        : d_fieldNames{fieldNames},
          d_child{child}
    {
    }

    const Tuple *ProjectionIterator::next()
    {
        const Tuple *nextTuple = d_child->next();
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
                return new Tuple{projectedFields};
            }
        }
        return nullptr;
    }

    void ProjectionIterator::close()
    {
    }
}
#include <tomekdb_limititerator.h>

namespace tomekdb
{

    LimitIterator::LimitIterator(size_t limit, Iterator *child) : d_limit{limit},
                                                                  d_child{child}
    {
    }

    const Tuple *LimitIterator::next()
    {
        if (d_limit)
        {
            --d_limit;
            return d_child->next();
        }
        return nullptr;
    }

    void LimitIterator::close()
    {
        d_child->close();
    }

}
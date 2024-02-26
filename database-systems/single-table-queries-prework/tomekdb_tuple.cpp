#include <tomekdb_tuple.h>
#include <algorithm>
#include <iterator>

namespace tomekdb
{
    Field::Field(const std::string &name,
                 const std::variant<int, double, std::string> &data) : d_name{name},
                                                                       d_data{data}
    {
    }

    std::ostream &operator<<(std::ostream &os, const tomekdb::Field &field)
    {
        os << "{ " << field.d_name << ", ";

        switch (field.d_data.index())
        {
        case 0:
            os << std::get<0>(field.d_data);
            break;
        case 1:
            os << std::get<1>(field.d_data);
            break;
        case 2:
            os << std::get<2>(field.d_data);
            break;
        default:
            break;
        }
        os << " }";
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const tomekdb::Tuple &tuple)
    {
        os << "[ ";
        std::copy(tuple.d_columns.begin(), tuple.d_columns.end(),
                  std::ostream_iterator<tomekdb::Field>(os, " "));
        os << "]\n";
        return os;
    }

}

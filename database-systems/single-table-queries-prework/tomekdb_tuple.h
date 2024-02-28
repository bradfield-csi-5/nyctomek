#ifndef TOMEKDB_TUPLE
#define TOMEKDB_TUPLE

#include <list>
#include <string>
#include <variant>
#include <initializer_list>
#include <iostream>

namespace tomekdb
{
  class Field
  {
  public:
    Field(const std::string &name,
          const std::variant<int, double, std::string> &data);

  const std::string& name() const;
  const std::variant<int, double, std::string>& data() const;

  private:
    std::string d_name;
    std::variant<int, double, std::string> d_data;

    friend std::ostream &operator<<(std::ostream &os, const Field &field);
  };

  class Tuple
  {
  public:
    Tuple(const std::list<Field> &columns) : d_columns{columns}
    {
    }

    Tuple(const std::initializer_list<Field> &columns) : d_columns{columns}
    {
    }

    const std::list<Field> &fields() const;

  private:
    std::list<Field> d_columns;

    friend std::ostream &operator<<(std::ostream &os, const Tuple &field);
  };
}

#endif

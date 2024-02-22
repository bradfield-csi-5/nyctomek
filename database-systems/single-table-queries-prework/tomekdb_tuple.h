#ifndef TOMEKDB_TUPLE
#define TOMEKDB_TUPLE

#include <list>
#include <string>
#include <variant>

class Field {
  std::string d_name;
  std::variant<int, double, string> d_data;
};

class Tuple {
  std::list<Field> d_columns;
};

#endif

#ifndef TUPLE
#define TUPLE

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

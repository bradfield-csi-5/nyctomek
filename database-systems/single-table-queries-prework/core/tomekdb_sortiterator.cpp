#include <tomekdb_sortiterator.h>
#include <algorithm>
#include <sstream>

namespace tomekdb {

SortIterator::SortIterator(const std::string &sortColumnName, SortIterator::SortOrder sortOrder, Iterator &child)
    : d_sortColumnName{sortColumnName},
      d_sortOrder{sortOrder},
      d_child{child}
      {}

std::optional<Tuple> SortIterator::next() {
    if(d_tuples.size() == 0) {
        while(std::optional<Tuple> tuple = d_child.next()) {
            bool sortKeyExists = false;
            for(const auto& column : tuple->fields()) {
                if(column.name() == d_sortColumnName) {
                    sortKeyExists = true;
                }
            }
            if(!sortKeyExists) {
                std::ostringstream os;
                os << "Sort key: " << d_sortColumnName << " does not exist in record: " << *tuple;
                throw std::runtime_error(os.str());
            }
            d_tuples.push_back(*tuple);
        }
    
        if(d_tuples.size() == 0) {
            return std::nullopt;
        }
        SortOrder sortOrder = d_sortOrder;
        std::sort(d_tuples.begin(), d_tuples.end(), [this](const Tuple& a, const Tuple& b) {
            const Field *fieldA = nullptr, *fieldB = nullptr;
            for(const auto &field : a.fields()) {
                if(field.name() == d_sortColumnName) {
                    fieldA = &field;
                }
            }
            for(const auto &field : b.fields()) {
                if(field.name() == d_sortColumnName) {
                    fieldB = &field;
                }
            }
            if(d_sortOrder == SortOrder::ASCENDING) {
                return fieldA->data() < fieldB->data();
            }
            else {
                return fieldA->data() > fieldB->data();
            }
        });
        d_currentRecord = d_tuples.begin();
    }
    if(d_currentRecord != d_tuples.end()) {
        return *d_currentRecord++;
    }
    return std::nullopt;
}

void SortIterator::close() {
}

}
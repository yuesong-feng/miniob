#pragma once
#include <vector>
#include "sql/expr/tuple.h"
#include "sql/expr/tuple_cell.h"

class ResTuple {
public:
    ResTuple();
    ~ResTuple();
    void add_cell(TupleCell &tuple_cell);
    void add_field(Field &field);
    int size();
    TupleCell &cell(int index);
    Field &field(int index);
    void set_fields(const std::vector<Field> &fields);
    std::vector<Field> &fields();

private:
    int null_bitmap;
    std::vector<Field> fields_;
    std::vector<TupleCell> tuple_;
};

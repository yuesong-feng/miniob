#include "sql/expr/res_tuple.h"

ResTuple::ResTuple() {}
ResTuple::~ResTuple() {}

void ResTuple::add_cell(TupleCell &cell) {
    tuple_.push_back(std::move(cell));
}

void ResTuple::add_field(Field &field) {
    fields_.push_back(std::move(field));
}

int ResTuple::size() { return tuple_.size(); }

TupleCell &ResTuple::cell(int index) { return tuple_[index]; }

Field &ResTuple::field(int index) { return fields_[index]; }

void ResTuple::set_fields(const std::vector<Field> &fields){
    fields_ = fields;
}

std::vector<Field> &ResTuple::fields(){
    return fields_;
}
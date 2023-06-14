/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "rc.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/filter_stmt.h"
#include <utility>
#include <vector>

class Table;

class UpdateStmt : public Stmt
{
public:

  UpdateStmt() = default;
  // UpdateStmt(Table *table, Value *values, int value_amount);
  // UpdateStmt(Table *table, Field field, std::vector<std::pair<Field, Value>> col_val, FilterStmt *filter_stmt);

public:
  static RC create(Db *db, const Updates &update_sql, Stmt *&stmt);

public:
  Table *table() const {return table_;}
  // Value *values() const { return values_; }
  // int value_amount() const { return value_amount_; }
  FilterStmt *filter_stmt() const { return filter_stmt_; }
  StmtType type() const override {
    return StmtType::UPDATE;
  }
  int col_val_size () const { return col_val_.size(); }
  Field &field(int index) {return col_val_[index].first;}
  Value &value(int index) {return col_val_[index].second;}
  void set_value(int index, Value &value) {col_val_[index].second = std::move(value);}
  // const char *attribute_name() const { return field_.field_name() ;}
  // Value value() const { return value_; }
  // Field field() const { return field_; }
  
private:
  Table *table_ = nullptr;
  std::vector<std::pair<Field, Value>> col_val_;
  // Field field_;
  // Value value_;
  FilterStmt *filter_stmt_ = nullptr;
};


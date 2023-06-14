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
// Created by Wangyunlai on 2022/6/5.
//

#pragma once

#include <cstddef>
#include <vector>

#include "rc.h"
#include "sql/stmt/stmt.h"
#include "storage/common/field.h"

class FieldMeta;
class FilterStmt;
class Db;
class Table;


class SelectStmt : public Stmt
{
public:

  SelectStmt() = default;
  ~SelectStmt() override;

  StmtType type() const override { return StmtType::SELECT; }
public:
  static RC create(Db *db, Selects &select_sql, Stmt *&stmt);

public:
  const std::vector<Table *> &tables() const { return tables_; }
  const std::vector<Field> &query_fields() const { return query_fields_; }
  const std::vector<Field> &order_by_fields() const { return order_by_fields_; }
  std::vector<RelAttr> &agg_fields() { return agg_fields_; }
  FilterStmt *filter_stmt() const { return filter_stmt_; }
  const std::vector<FilterStmt *> &join_filters() const { return join_filters_; }
  const std::vector<Field> &group_by_fields() const { return group_by_fields_; }
  const std::vector<Subset> &subsets() const {return subsets_; }
  void add_subset(Subset &subset) {subsets_.push_back(subset); }
  const std::vector<Subquery> &subquery() const {return subquery_; }
  FilterStmt *having_filter() const { return having_filter_; }
private:
  std::vector<Field> query_fields_;
  std::vector<RelAttr> agg_fields_;
  std::vector<Table *> tables_;
  FilterStmt *filter_stmt_ = nullptr;
  std::vector<FilterStmt *> join_filters_;
  std::vector<Field> order_by_fields_;
  std::vector<Field> group_by_fields_;
  std::vector<Subset> subsets_;
  std::vector<Subquery> subquery_;
  FilterStmt *having_filter_ = nullptr;
};


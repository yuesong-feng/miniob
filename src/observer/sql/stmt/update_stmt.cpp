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

#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "storage/common/db.h"
#include "storage/common/field.h"
#include "storage/common/field_meta.h"
#include "storage/common/table.h"
#include <utility>

// UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount)
//   : table_ (table), values_(values), value_amount_(value_amount)
// {}

// UpdateStmt::UpdateStmt(Table *table, Field field, const Value &value, FilterStmt *filter_stmt)
//   :table_(table), field_(field), value_(value), filter_stmt_(filter_stmt)
// {}

RC UpdateStmt::create(Db *db, const Updates &update, Stmt *&stmt)
{
  if (nullptr == db || nullptr == update.relation_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, update.relation_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  const char *table_name = update.relation_name;
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // check whether the field exists
  const TableMeta &table_meta = table->table_meta();
  for (int i = 0; i < update.attribute_count; ++i) {
    const char *field_name = update.attributes[i];
    const FieldMeta *field_meta = table_meta.field(field_name);
    if(nullptr == field_meta) {
      LOG_WARN("field %s does not exists on table %s, db=%s", field_name ,db->name(), table_name);
      return RC::SCHEMA_FIELD_NOT_EXIST;
    }
  }

  // check whether the field num = value num
  if (update.attribute_count != update.value_num) {
    LOG_WARN("should not happen");
    return RC::INTERNAL;
  }

  FilterStmt *filter_stmt = nullptr;
  // if the update is conditional
  if(update.condition_num >= 1) {
    std::unordered_map<std::string, Table *> table_map;
    table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));

    RC rc = FilterStmt::create(db, table, &table_map, 
              update.conditions, update.condition_num, filter_stmt);
    if(rc != RC::SUCCESS) {
      LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
  }

  std::vector<std::pair<Field, Value>> col_val;
  // Field field(table, field_meta);
  for (int i = 0; i < update.attribute_count; ++i) {
    Field field(table, table_meta.field(update.attributes[i]));
    Value value = update.values[i];
    std::pair<Field, Value> pair = {field, value};
    col_val.push_back(pair);
  }

  UpdateStmt *update_stmt = new UpdateStmt();
  update_stmt->table_ = table;
  update_stmt->filter_stmt_ = filter_stmt;
  update_stmt->col_val_.swap(col_val);
  stmt = update_stmt;
  return RC::SUCCESS;
}

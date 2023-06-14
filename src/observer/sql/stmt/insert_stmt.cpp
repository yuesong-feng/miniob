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

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "sql/parser/parse_defs.h"
#include "storage/common/db.h"
#include "storage/common/table.h"
#include "util/util.h"
#include <cstring>
#include <string.h>

InsertStmt::InsertStmt(Table *table, const Value *values, int value_amount)
  : table_ (table), values_(values), value_amount_(value_amount)
{}

RC InsertStmt::create(Db *db, const Inserts &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name;
  if (nullptr == db || nullptr == table_name || inserts.value_num <= 0) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d", 
             db, table_name, inserts.value_num);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // check the fields number
  const Value *values = inserts.values;
  const int value_num = inserts.value_num;
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num() - table_meta.sys_field_num();
  if (value_num % field_num != 0) {
    LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
    return RC::SCHEMA_FIELD_MISSING;
  }

  // check fields type
  const int sys_field_num = table_meta.sys_field_num();
  for(int t = 0; t < value_num; t += field_num) {
    for (int i = t; i < t + field_num; i++) {
      const FieldMeta *field_meta = table_meta.field((i % field_num) + sys_field_num);
      const AttrType field_type = field_meta->type();
      const AttrType value_type = values[i].type;

      if (field_type == INTS && value_type == CHARS) { // TODO try to convert the value type to field type
        char* val_chars = (char*)values[i].data;
        int* val_ints = chars2ints(val_chars);
        Value *value2 = new Value();
        value2->type = INTS;
        value2->data = val_ints;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if (field_type == CHARS && value_type == INTS) {
        int* val_ints = (int*)values[i].data;
        char* val_chars = ints2chars(val_ints);
        Value *value2 = new Value();
        value2->type = CHARS;
        value2->data = val_chars;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if (field_type == FLOATS && value_type == CHARS) {
        char* val_chars = (char*)values[i].data;
        float* val_floats = chars2floats(val_chars);
        Value *value2 = new Value();
        value2->type = FLOATS;
        value2->data = val_floats;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if (field_type == CHARS && value_type == FLOATS) {
        float* val_floats = (float*)values[i].data;
        char* val_chars = floats2chars(val_floats);
        Value *value2 = new Value();
        value2->type = CHARS;
        value2->data = val_chars;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if (field_type == FLOATS && value_type == INTS) {
        int* val_ints = (int*)values[i].data;
        float* val_floats = ints2floats(val_ints);
        Value *value2 = new Value();
        value2->type = FLOATS;
        value2->data = val_floats;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if (field_type == INTS && value_type == FLOATS) {
        float* val_floats = (float*)values[i].data;
        int* val_ints = floats2ints(val_floats);
        Value *value2 = new Value();
        value2->type = INTS;
        value2->data = val_ints;
        *(Value*)(values + i) = *(value2);
        // TBD: 释放内存
      }
      else if(field_type == TEXTS && value_type == CHARS) {
        Value *value2 = new Value();
        value2->type = TEXTS;
        char *texts = new char[4097];
        memset(texts, 0, 4097);
        size_t copy_len = strlen((char *)values[i].data);
        if (copy_len > 4096) {
          copy_len = 4096;
        }
        memcpy(texts, values[i].data, copy_len);
        value2->data = texts;
        *(Value*)(values + i) = *(value2);
      }
      else if (value_type == NULLS) {
        // value：null，检查field是否是nullable
        if (field_meta->nullable() == false) {
          LOG_WARN("field %s can not be null", field_meta->name());
          return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
      }
      else if (field_type != value_type) { 
        LOG_WARN("field type mismatch. table=%s, field=%s, field type=%d, value_type=%d", 
          table_name, field_meta->name(), field_type, value_type);
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
    }
  }

  // everything alright
  stmt = new InsertStmt(table, values, value_num);
  return RC::SUCCESS;
}

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
// Created by WangYunlai on 2021/6/9.
//

#include "sql/operator/insert_operator.h"
#include "sql/stmt/insert_stmt.h"
#include "storage/common/table.h"
#include "rc.h"

RC InsertOperator::open()
{
  Table *table = insert_stmt_->table();
  const Value *values = insert_stmt_->values();
  int value_amount = insert_stmt_->value_amount();
  int field_num = table->table_meta().field_num() - table->table_meta().sys_field_num();
  if(field_num == value_amount) {  // 单条插入
    return table->insert_record(trx_, value_amount, values);
    // return table->insert_record(nullptr, value_amount, values);
  } else if(value_amount % field_num == 0) {  // 多条插入
    for(int i = 0; i < value_amount; i += field_num) {
      RC rc = table->insert_record(trx_, field_num, values + i);
      // RC rc = table->insert_record(nullptr, field_num, values + i);
      if(rc != SUCCESS) {
        LOG_WARN("Insert multiple record failed");
        return RC::INTERNAL;
      }
    }
    return RC::SUCCESS;
  } else {
    LOG_ERROR("Something wrong happend in InsertOperator::open");
    return RC::SCHEMA_FIELD_MISSING;
  }
}

RC InsertOperator::next()
{
  return RC::RECORD_EOF;
}

RC InsertOperator::close()
{
  return RC::SUCCESS;
}

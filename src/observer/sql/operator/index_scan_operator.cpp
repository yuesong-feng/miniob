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
// Created by Wangyunlai on 2022/07/08.
//

#include "sql/operator/index_scan_operator.h"
#include "storage/index/index.h"

IndexScanOperator::IndexScanOperator(const Table *table, Index *index,
		std::vector<TupleCell> left_cells, bool left_inclusive,
		std::vector<TupleCell> right_cells, bool right_inclusive)
  : table_(table), index_(index), left_cells_(left_cells), right_cells_(right_cells),
    left_inclusive_(left_inclusive), right_inclusive_(right_inclusive)
{
  // if (left_cell) {
  //   left_cell_ = *left_cell;
  // }
  // if (right_cell) {
  //   right_cell_ = *right_cell;
  // }
}

RC IndexScanOperator::open()
{
  if (nullptr == table_ || nullptr == index_) {
    return RC::INTERNAL;
  }

  // TODO: 这里的left_cells_data和right_cells_data应该和index的索引列一致
  char *left_cells_data = nullptr, *right_cells_data = nullptr;
  int left_length = 0, right_length = 0;
  if (left_cells_.size() > 0) {
    for (auto left_cell : left_cells_) {
      left_length += left_cell.length();
    }
    left_cells_data = new char[left_cells_.size() * 4 + 1];
    memset(left_cells_data, 0, sizeof(char) * (left_length+1));
    int left_start_idx = 0;
    for (auto left_cell : left_cells_) {
      memcpy(left_cells_data+left_start_idx, left_cell.data(), left_cell.length());
      left_start_idx += left_cell.length();
    }
  }
  if (right_cells_.size() > 0) {
    for (auto right_cell : right_cells_) {
      right_length += right_cell.length();
    }
    right_cells_data = new char[right_cells_.size()*4 + 1];
    memset(right_cells_data, 0, sizeof(char) * (right_length+1));
    int right_start_idx = 0;
    for (auto right_cell : right_cells_) {
      memcpy(right_cells_data+right_start_idx, right_cell.data(), right_cell.length());
      right_start_idx += right_cell.length();
    }
  }

  IndexScanner *index_scanner = index_->create_scanner(left_cells_data, left_length, left_inclusive_,
                                                       right_cells_data, right_length, right_inclusive_);
  if (nullptr == index_scanner) {
    LOG_WARN("failed to create index scanner");
    return RC::INTERNAL;
  }

  record_handler_ = table_->record_handler();
  if (nullptr == record_handler_) {
    LOG_WARN("invalid record handler");
    index_scanner->destroy();
    return RC::INTERNAL;
  }
  index_scanner_ = index_scanner;

  tuple_.set_schema(table_, table_->table_meta().field_metas());
  
  return RC::SUCCESS;
}

RC IndexScanOperator::next()
{
  RID rid;
  RC rc = index_scanner_->next_entry(&rid);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return record_handler_->get_record(&rid, &current_record_);
}

RC IndexScanOperator::close()
{
  index_scanner_->destroy();
  index_scanner_ = nullptr;
  return RC::SUCCESS;
}

Tuple * IndexScanOperator::current_tuple()
{
  tuple_.set_record(&current_record_);
  return &tuple_;
}

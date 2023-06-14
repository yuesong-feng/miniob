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
// Created by Meiyi & Wangyunlai on 2021/5/13.
//

#include <limits.h>
#include <string.h>
#include <algorithm>

#include "common/defs.h"
#include "storage/common/table.h"
#include "storage/common/table_meta.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/default/disk_buffer_pool.h"
#include "storage/record/record_manager.h"
#include "storage/common/condition_filter.h"
#include "storage/common/meta_util.h"
#include "storage/index/index.h"
#include "storage/index/bplus_tree_index.h"
#include "storage/trx/trx.h"
#include "storage/clog/clog.h"

#include "sql/expr/tuple.h"
#include "sql/operator/operator.h"
#include "sql/operator/table_scan_operator.h"
#include "sql/operator/index_scan_operator.h"
#include "sql/operator/predicate_operator.h"
#include "sql/stmt/filter_stmt.h"

IndexScanOperator *create_index_scan_operator_by_index(FilterStmt *filter_stmt, Index *index);

Table::~Table()
{
  if (record_handler_ != nullptr) {
    delete record_handler_;
    record_handler_ = nullptr;
  }

  if (data_buffer_pool_ != nullptr) {
    data_buffer_pool_->close_file();
    data_buffer_pool_ = nullptr;
  }

  for (std::vector<Index *>::iterator it = indexes_.begin(); it != indexes_.end(); ++it) {
    Index *index = *it;
    delete index;
  }
  indexes_.clear();

  LOG_INFO("Table has been closed: %s", name());
}

RC Table::create(
    const char *path, const char *name, const char *base_dir, int attribute_count, const AttrInfo attributes[], CLogManager *clog_manager)
{

  if (common::is_blank(name)) {
    LOG_WARN("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }
  LOG_INFO("Begin to create table %s:%s", base_dir, name);

  if (attribute_count <= 0 || nullptr == attributes) {
    LOG_WARN("Invalid arguments. table_name=%s, attribute_count=%d, attributes=%p", name, attribute_count, attributes);
    return RC::INVALID_ARGUMENT;
  }

  RC rc = RC::SUCCESS;

  // 使用 table_name.table记录一个表的元数据
  // 判断表文件是否已经存�?
  int fd = ::open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
  if (fd < 0) {
    if (EEXIST == errno) {
      LOG_ERROR("Failed to create table file, it has been created. %s, EEXIST, %s", path, strerror(errno));
      return RC::SCHEMA_TABLE_EXIST;
    }
    LOG_ERROR("Create table file failed. filename=%s, errmsg=%d:%s", path, errno, strerror(errno));
    return RC::IOERR;
  }

  close(fd);

  // 创建文件
  if ((rc = table_meta_.init(name, attribute_count, attributes)) != RC::SUCCESS) {
    LOG_ERROR("Failed to init table meta. name:%s, ret:%d", name, rc);
    return rc;  // delete table file
  }

  std::fstream fs;
  fs.open(path, std::ios_base::out | std::ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", path, strerror(errno));
    return RC::IOERR;
  }

  // 记录元数据到文件�?
  table_meta_.serialize(fs);
  fs.close();

  std::string data_file = table_data_file(base_dir, name);
  BufferPoolManager &bpm = BufferPoolManager::instance();
  rc = bpm.create_file(data_file.c_str());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create disk buffer pool of data file. file name=%s", data_file.c_str());
    return rc;
  }

  rc = init_record_handler(base_dir);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create table %s due to init record handler failed.", data_file.c_str());
    // don't need to remove the data_file
    return rc;
  }

  base_dir_ = base_dir;
  clog_manager_ = clog_manager;
  LOG_INFO("Successfully create table %s:%s", base_dir, name);
  return rc;
}

RC Table::open(const char *meta_file, const char *base_dir, CLogManager *clog_manager)
{
  // 加载元数据文�?
  std::fstream fs;
  std::string meta_file_path = std::string(base_dir) + common::FILE_PATH_SPLIT_STR + meta_file;
  fs.open(meta_file_path, std::ios_base::in | std::ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open meta file for read. file name=%s, errmsg=%s", meta_file_path.c_str(), strerror(errno));
    return RC::IOERR;
  }
  if (table_meta_.deserialize(fs) < 0) {
    LOG_ERROR("Failed to deserialize table meta. file name=%s", meta_file_path.c_str());
    fs.close();
    return RC::GENERIC_ERROR;
  }
  fs.close();

  // 加载数据文件
  RC rc = init_record_handler(base_dir);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to open table %s due to init record handler failed.", base_dir);
    // don't need to remove the data_file
    return rc;
  }

  base_dir_ = base_dir;

  const int index_num = table_meta_.index_num();
  for (int i = 0; i < index_num; i++) {
    const IndexMeta *index_meta = table_meta_.index(i);
    // const FieldMeta *field_meta = table_meta_.field(index_meta->field());
    std::vector<FieldMeta> field_metas;
    for (auto field_name : index_meta->fields()){
      field_metas.push_back(*table_meta_.field(field_name));
    }
    if (field_metas.size()==0) {
      LOG_ERROR("Found invalid index meta info which has a non-exists field. table=%s, index=%s, field=%s",
          name(),
          index_meta->name(),
          index_meta->field2());
      // skip cleanup
      //  do all cleanup action in destructive Table function
      return RC::GENERIC_ERROR;
    }

    BplusTreeIndex *index = new BplusTreeIndex();
    std::string index_file = table_index_file(base_dir, name(), index_meta->name());
    rc = index->open(index_file.c_str(), *index_meta, field_metas);
    if (rc != RC::SUCCESS) {
      delete index;
      LOG_ERROR("Failed to open index. table=%s, index=%s, file=%s, rc=%d:%s",
          name(),
          index_meta->name(),
          index_file.c_str(),
          rc,
          strrc(rc));
      // skip cleanup
      //  do all cleanup action in destructive Table function.
      return rc;
    }
    indexes_.push_back(index);
  }

  if (clog_manager_ == nullptr) {
    clog_manager_ = clog_manager;
  }
  return rc;
}

RC Table::drop(const char *base_dir){
  LOG_INFO("Begin to drop table %s:%s", base_dir, name());

  RC rc = sync(); // 刷新所有脏�?
  if(rc != RC::SUCCESS) return rc;

  // 删除索引
  for(Index *index : indexes_) {
    BplusTreeIndex *btidx = (BplusTreeIndex *)index;
    btidx->close();
    std::string index_file = table_index_file(base_dir, name(), btidx->index_meta().name());
    if(unlink(index_file.c_str()) != 0) {
      LOG_ERROR("Failed to remove meta file=%s, errno=%d", index_file.c_str(), errno);
      return RC::IOERR_DELETE;
    }
  }
  std::string meta_file = table_meta_file(base_dir, name());
  std::string data_file = table_data_file(base_dir, name());

  // BufferPoolManager &bpm = BufferPoolManager::instance();
  // rc = bpm.close_file(data_file.c_str());
  // if (rc != RC::SUCCESS) {
  //   LOG_ERROR("Failed to create disk buffer pool of data file. file name=%s", data_file.c_str());
  //   return rc;
  // }

  // 删除元数据文�?
  if(unlink(meta_file.c_str()) != 0) {
      LOG_ERROR("Failed to remove meta file=%s, errno=%d", meta_file.c_str(), errno);
      return RC::IOERR_DELETE;
  }
  
  // 删除数据文件
  if(unlink(data_file.c_str()) != 0) {
      LOG_ERROR("Failed to remove meta file=%s, errno=%d", data_file.c_str(), errno);
      return RC::IOERR_DELETE;
  }

  return rc;
}

RC Table::commit_insert(Trx *trx, const RID &rid)
{
  Record record;
  RC rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to get record %s: %s", this->name(), rid.to_string().c_str());
    return rc;
  }

  return trx->commit_insert(this, record);
}

RC Table::commit_update(Trx *trx, const RID &rid) {
  Record record;
  RC rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to get record %s: %s", this->name(), rid.to_string().c_str());
    return rc;
  }

  return trx->commit_update(this, record);
}

RC Table::rollback_insert(Trx *trx, const RID &rid)
{

  Record record;
  RC rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to get record %s: %s", this->name(), rid.to_string().c_str());
    return rc;
  }

  // remove all indexes
  rc = delete_entry_of_indexes(record.data(), rid, false);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete indexes of record(rid=%d.%d) while rollback insert, rc=%d:%s",
        rid.page_num,
        rid.slot_num,
        rc,
        strrc(rc));
    return rc;
  }

  rc = record_handler_->delete_record(&rid);
  return rc;
}

RC Table::insert_record(Trx *trx, Record *record)
{
  RC rc = RC::SUCCESS;

  // 判断是否违反unique索引, 方法1：全表扫描，将每条tuple和待插入的tuple比较，看是否有unique字段重复
  // const int normal_field_start_index = table_meta_.sys_field_num();
  // std::vector<bool> need_check_unique(value_num, false);
  // for (int i = 0; i < value_num; ++i) {
  //   const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
  //   if(find_index_by_field(field->name())!=nullptr && find_index_by_field(field->name())->index_meta().is_unique()){
  //     need_check_unique[i] = true;
  //   }
  // }
  // Operator *scan_oper = new TableScanOperator(this);
  // rc = scan_oper->open();
  // if (rc != RC::SUCCESS) {
  //   LOG_WARN("failed to open operator");
  //   return rc;
  // }
  // while ((rc = scan_oper->next()) == RC::SUCCESS) {
  //   // get current record
  //   Tuple * tuple = scan_oper->current_tuple();
  //   if (nullptr == tuple) {
  //     rc = RC::INTERNAL;
  //     LOG_WARN("failed to get current record. rc=%s", strrc(rc));
  //     break;
  //   }
  //   TupleCell cell;
  //   for (int i = 0; i < tuple->cell_num(); ++i) {
  //     rc = tuple->cell_at(i, cell);
  //     if (rc != RC::SUCCESS) {
  //       LOG_WARN("failed to fetch field of cell. index=%d, rc=%s", i, strrc(rc));
  //       return RC::INTERNAL;
  //     }
  //     if (need_check_unique[i]) {
  //       switch (cell.attr_type()) {
  //       case INTS: 
  //       case DATES: {
  //         if (compare_int((void *)cell.data(), (void *)values[i].data)==0) rc = RC::INTERNAL;
  //         break;
  //       }
  //       case FLOATS: {
  //         if (compare_float((void *)cell.data(), (void *)values[i].data)==0) rc = RC::INTERNAL;
  //         break;
  //       }
  //       case CHARS: {
  //         if (compare_string((void *)cell.data(), cell.length(), (void *)values[i].data, strlen((char*)values[i].data))==0) rc = RC::INTERNAL;
  //         break;
  //       }
  //       if(rc != RC::SUCCESS){
  //         LOG_ERROR("Unique index violated. rc=%d:%s", rc, strrc(rc));
  //         return rc;
  //       }
  //       }
  //     }
  //   }
  // }

  // 判断是否违反unique索引, 方法2：对于每个unique索引，创建filter_units，并使用unique索引进行扫描，看是否有结果输�?
  const int normal_field_start_index = table_meta_.sys_field_num();
  const int value_num = table_meta_.field_num() - table_meta_.sys_field_num();
  for(Index *index : indexes_){
    if(index->index_meta().is_unique()){
      FilterStmt *filter_stmt = new FilterStmt();
      std::vector<int> attr_idxs;
      for(int i=0; i<index->field_metas().size(); ++i) {
        CompOp comp = EQUAL_TO;
        Expression *left = new FieldExpr(this, &index->field_metas()[i]);
        int right_idx = -1;
        for(int j=0; j<value_num; ++j){
          if(strcmp(table_meta_.field(j + normal_field_start_index)->name(), index->index_meta().fields(i))==0){
            right_idx = j;
            attr_idxs.push_back(j);
            break;
          }
        }
        if(right_idx==-1){
          LOG_ERROR("the field of index not exit in this table");
        }
        const FieldMeta *field = table_meta_.field(right_idx + normal_field_start_index);
        char *value = new char[field->len()+1];
        memset(value, 0, sizeof(char) * (field->len()+1));
        memcpy(value, record->data() + field->offset(), field->len());
        TupleCell cell(field->type(), value);
        cell.set_length(field->len());
        Expression *right = new ValueExpr(cell);
        FilterUnit *filter_unit = new FilterUnit();
        filter_unit->set_comp(comp);
        filter_unit->set_left(left);
        filter_unit->set_right(right);
        filter_stmt->add_unit(filter_unit);
      }
      Operator *scan_oper = create_index_scan_operator_by_index(filter_stmt, index);
      if (nullptr == scan_oper) {
        scan_oper = new TableScanOperator(this);
      }
      PredicateOperator pred_oper(filter_stmt);
      pred_oper.add_child(scan_oper);
      rc = pred_oper.open();
      if (rc != RC::SUCCESS) {
        // LOG_WARN("Failed to open operator. rc=%d:%s", rc, strrc(rc));
        // return rc;
        // TODO: 这里open失败是由于表中没有数据，index没有root_page，所以直接continue判断下一个index，逻辑不严�?
        continue;
      }
      if(pred_oper.next() == RC::SUCCESS){
        Tuple * tuple = pred_oper.current_tuple();
        TupleCell null_cell;
        rc = tuple->cell_at(NULLABLE_COLUMN, null_cell);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to fetch _null field of cell. rc=%s", strrc(rc));
          return RC::INTERNAL;
        }
        const char* null_data = null_cell.data();
        
        TupleCell cell;

        bool have_null = false;
        for (int i = 0; i < attr_idxs.size(); ++i) {
          rc = tuple->cell_at(i+normal_field_start_index, cell);
          if (rc != RC::SUCCESS) {
            LOG_WARN("failed to fetch field of cell. index=%d, rc=%s", i, strrc(rc));
            return RC::INTERNAL;
          }
          if ((null_data[(attr_idxs[i])/BYTE_BIT] & (0x01 << (BYTE_MOVE_BIT-(attr_idxs[i])%BYTE_BIT))) != 0) {
            have_null = true;
          }
        }
        if (!have_null){
          LOG_ERROR("Unique index %s violated", index->index_meta().name());
          return RC::INTERNAL;
        }
      }
    }
  }

  if (trx != nullptr) {
    trx->init_trx_info(this, *record);
  }  
  rc = record_handler_->insert_record(record->data(), table_meta_.record_size(), &record->rid());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Insert record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return rc;
  }

  if (trx != nullptr) {
    rc = trx->insert_record(this, record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to log operation(insertion) to trx");

      RC rc2 = record_handler_->delete_record(&record->rid());
      if (rc2 != RC::SUCCESS) {
        LOG_ERROR("Failed to rollback record data when insert index entries failed. table name=%s, rc=%d:%s",
            name(),
            rc2,
            strrc(rc2));
      }
      return rc;
    }
  }

  rc = insert_entry_of_indexes(record->data(), record->rid());
  if (rc != RC::SUCCESS) {
    RC rc2 = delete_entry_of_indexes(record->data(), record->rid(), true);
    if (rc2 != RC::SUCCESS) {
      LOG_ERROR("Failed to rollback index data when insert index entries failed. table name=%s, rc=%d:%s",
          name(),
          rc2,
          strrc(rc2));
    }
    rc2 = record_handler_->delete_record(&record->rid());
    if (rc2 != RC::SUCCESS) {
      LOG_PANIC("Failed to rollback record data when insert index entries failed. table name=%s, rc=%d:%s",
          name(),
          rc2,
          strrc(rc2));
    }
    return rc;
  }

  if (trx != nullptr) {
    // append clog record
    CLogRecord *clog_record = nullptr;
    rc = clog_manager_->clog_gen_record(CLogType::REDO_INSERT, trx->get_current_id(), clog_record, name(), table_meta_.record_size(), record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create a clog record. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
    rc = clog_manager_->clog_append_record(clog_record);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }
  return rc;
}

RC Table::recover_insert_record(Record *record)
{
  RC rc = RC::SUCCESS;

  rc = record_handler_->recover_insert_record(record->data(), table_meta_.record_size(), &record->rid());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Insert record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return rc;
  }

  return rc;
}

RC Table::insert_record(Trx *trx, int value_num, const Value *values)
{
  if (value_num <= 0 || nullptr == values) {
    LOG_ERROR("Invalid argument. table name: %s, value num=%d, values=%p", name(), value_num, values);
    return RC::INVALID_ARGUMENT;
  }

  // 检查字段类型是否一�?
  if (value_num + table_meta_.sys_field_num() != table_meta_.field_num()) {
    LOG_WARN("Input values don't match the table's schema, table name:%s", table_meta_.name());
    return RC::SCHEMA_FIELD_MISSING;
  }

  char *record_data;
  RC rc = make_record(value_num, values, record_data);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create a record. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  Record record;
  record.set_data(record_data);
  rc = insert_record(trx, &record);
  delete[] record_data;
  return rc;
}

const char *Table::name() const
{
  return table_meta_.name();
}

const TableMeta &Table::table_meta() const
{
  return table_meta_;
}

RC Table::make_record(int value_num, const Value *values, char *&record_out)
{
  // 检查字段类型是否一�?
  if (value_num + table_meta_.sys_field_num() != table_meta_.field_num()) {
    LOG_WARN("Input values don't match the table's schema, table name:%s", table_meta_.name());
    return RC::SCHEMA_FIELD_MISSING;
  }

  const int normal_field_start_index = table_meta_.sys_field_num();
  const FieldMeta *nullable_meta = table_meta_.field("_nullable");
  for (int i = 0; i < value_num; i++) {
    const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
    const Value &value = values[i];
    if (value.type == NULLS) {
      // return RC::UNIMPLENMENT; // TODO: remove
      if (field->nullable() == false) {
        LOG_WARN("Field %s can not be null", field->name());
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
    }
    else if (field->type() != value.type) {
      LOG_ERROR("Invalid value type. table name =%s, field name=%s, type=%d, but given=%d",
          table_meta_.name(),
          field->name(),
          field->type(),
          value.type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

  // 复制所有字段的值，未处理空�?
  int record_size = table_meta_.record_size();
  char *record = new char[record_size];
  char *null_bitmap = new char[nullable_meta->len()];
  memset(null_bitmap, 0, nullable_meta->len());
  for (int i = 0; i < value_num; i++) {
    const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
    const Value &value = values[i];
    size_t copy_len = field->len();
    if (value.type != NULLS && field->type() == CHARS) {
      const size_t data_len = strlen((const char *)value.data);
      if (copy_len > data_len) {
        copy_len = data_len + 1;
      }
    }
    if (value.type == NULLS && field->nullable() == true) {
      uint8_t mask = (0x01 << (BYTE_MOVE_BIT - i % BYTE_BIT));
      null_bitmap[i / BYTE_BIT] |= mask;
      char *null_data = new char[field->len()];
      memset(null_data, 0, field->len());
      memcpy(record + field->offset(), null_data, field->len());
      delete []null_data;
      continue;
    }
    memcpy(record + field->offset(), value.data, copy_len);
  }
  memcpy(record + nullable_meta->offset(), null_bitmap, nullable_meta->len());
  delete [] null_bitmap;
  record_out = record;
  return RC::SUCCESS;
}

RC Table::init_record_handler(const char *base_dir)
{
  std::string data_file = table_data_file(base_dir, table_meta_.name());

  RC rc = BufferPoolManager::instance().open_file(data_file.c_str(), data_buffer_pool_);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to open disk buffer pool for file:%s. rc=%d:%s", data_file.c_str(), rc, strrc(rc));
    return rc;
  }

  record_handler_ = new RecordFileHandler();
  rc = record_handler_->init(data_buffer_pool_);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to init record handler. rc=%d:%s", rc, strrc(rc));
    data_buffer_pool_->close_file();
    data_buffer_pool_ = nullptr;
    delete record_handler_;
    record_handler_ = nullptr;
    return rc;
  }

  return rc;
}

RC Table::get_record_scanner(RecordFileScanner &scanner)
{
  RC rc = scanner.open_scan(*data_buffer_pool_, nullptr);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("failed to open scanner. rc=%d:%s", rc, strrc(rc));
  }
  return rc;
}

/**
 * 为了不把Record暴露出去，封装一�?
 */
class RecordReaderScanAdapter {
public:
  explicit RecordReaderScanAdapter(void (*record_reader)(const char *data, void *context), void *context)
      : record_reader_(record_reader), context_(context)
  {}

  void consume(const Record *record)
  {
    record_reader_(record->data(), context_);
  }

private:
  void (*record_reader_)(const char *, void *);
  void *context_;
};

static RC scan_record_reader_adapter(Record *record, void *context)
{
  RecordReaderScanAdapter &adapter = *(RecordReaderScanAdapter *)context;
  adapter.consume(record);
  return RC::SUCCESS;
}

RC Table::scan_record(Trx *trx, ConditionFilter *filter,
		      int limit, void *context,
		      void (*record_reader)(const char *data, void *context))
{
  RecordReaderScanAdapter adapter(record_reader, context);
  return scan_record(trx, filter, limit, (void *)&adapter, scan_record_reader_adapter);
}

RC Table::scan_record(Trx *trx, ConditionFilter *filter, int limit, void *context,
                      RC (*record_reader)(Record *record, void *context))
{
  if (nullptr == record_reader) {
    return RC::INVALID_ARGUMENT;
  }

  if (0 == limit) {
    return RC::SUCCESS;
  }

  if (limit < 0) {
    limit = INT_MAX;
  }

  IndexScanner *index_scanner = find_index_for_scan(filter);
  if (index_scanner != nullptr) {
    return scan_record_by_index(trx, index_scanner, filter, limit, context, record_reader);
  }

  RC rc = RC::SUCCESS;
  RecordFileScanner scanner;
  rc = scanner.open_scan(*data_buffer_pool_, filter);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("failed to open scanner. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  int record_count = 0;
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to fetch next record. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
    if (trx == nullptr || trx->is_visible(this, &record)) {
      rc = record_reader(&record, context);
      if (rc != RC::SUCCESS) {
        break;
      }
      record_count++;
    }
  }

  scanner.close_scan();
  return rc;
}

RC Table::scan_record_by_index(Trx *trx, IndexScanner *scanner, ConditionFilter *filter,
                               int limit, void *context,
                               RC (*record_reader)(Record *, void *))
{
  RC rc = RC::SUCCESS;
  RID rid;
  Record record;
  int record_count = 0;
  while (record_count < limit) {
    rc = scanner->next_entry(&rid);
    if (rc != RC::SUCCESS) {
      if (RC::RECORD_EOF == rc) {
        rc = RC::SUCCESS;
        break;
      }
      LOG_ERROR("Failed to scan table by index. rc=%d:%s", rc, strrc(rc));
      break;
    }

    rc = record_handler_->get_record(&rid, &record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to fetch record of rid=%d:%d, rc=%d:%s", rid.page_num, rid.slot_num, rc, strrc(rc));
      break;
    }

    if ((trx == nullptr || trx->is_visible(this, &record)) && (filter == nullptr || filter->filter(record))) {
      rc = record_reader(&record, context);
      if (rc != RC::SUCCESS) {
        LOG_TRACE("Record reader break the table scanning. rc=%d:%s", rc, strrc(rc));
        break;
      }
    }

    record_count++;
  }

  scanner->destroy();
  return rc;
}

class IndexInserter {
public:
  explicit IndexInserter(Index *index) : index_(index)
  {}

  RC insert_index(const Record *record)
  {
    return index_->insert_entry(record->data(), &record->rid());
  }

private:
  Index *index_;
};

static RC insert_index_record_reader_adapter(Record *record, void *context)
{
  IndexInserter &inserter = *(IndexInserter *)context;
  return inserter.insert_index(record);
}

RC Table::create_index(Trx *trx, const char *index_name, const char *attribute_name, int is_unique)
{
  if (common::is_blank(index_name) || common::is_blank(attribute_name)) {
    LOG_INFO("Invalid input arguments, table name is %s, index_name is blank or attribute_name is blank", name());
    return RC::INVALID_ARGUMENT;
  }
  std::vector<const char*> fields;
  fields.push_back(attribute_name);
  if (table_meta_.index(index_name) != nullptr || table_meta_.find_index_by_fields(fields)) {
    LOG_INFO("Invalid input arguments, table name is %s, index %s exist or attribute %s exist index",
             name(), index_name, attribute_name);
    return RC::SCHEMA_INDEX_EXIST;
  }

  std::vector<FieldMeta> field_metas;
  const FieldMeta *field_meta = table_meta_.field(attribute_name);
  if (!field_meta) {
    LOG_INFO("Invalid input arguments, there is no field of %s in table:%s.", attribute_name, name());
    return RC::SCHEMA_FIELD_MISSING;
  }
  field_metas.push_back(*field_meta);

  IndexMeta new_index_meta;
  RC rc = new_index_meta.init(index_name, field_metas, is_unique == 1);
  if (rc != RC::SUCCESS) {
    LOG_INFO("Failed to init IndexMeta in table:%s, index_name:%s, field_name:%s",
             name(), index_name, attribute_name);
    return rc;
  }

  // 创建索引相关数据
  BplusTreeIndex *index = new BplusTreeIndex();
  std::string index_file = table_index_file(base_dir_.c_str(), name(), index_name);
  rc = index->create(index_file.c_str(), new_index_meta, field_metas);
  if (rc != RC::SUCCESS) {
    delete index;
    LOG_ERROR("Failed to create bplus tree index. file name=%s, rc=%d:%s", index_file.c_str(), rc, strrc(rc));
    return rc;
  }

  // 遍历当前的所有数据，插入这个索引
  // TODO: 唯一索引实现，发现表的索引列有重复数据时，失�?
  IndexInserter index_inserter(index);
  rc = scan_record(trx, nullptr, -1, &index_inserter, insert_index_record_reader_adapter);
  if (rc != RC::SUCCESS) {
    // rollback
    delete index;
    LOG_ERROR("Failed to insert index to all records. table=%s, rc=%d:%s", name(), rc, strrc(rc));
    return rc;
  }
  indexes_.push_back(index);

  TableMeta new_table_meta(table_meta_);
  rc = new_table_meta.add_index(new_index_meta);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to add index (%s) on table (%s). error=%d:%s", index_name, name(), rc, strrc(rc));
    return rc;
  }
  // 创建元数据临时文�?
  std::string tmp_file = table_meta_file(base_dir_.c_str(), name()) + ".tmp";
  std::fstream fs;
  fs.open(tmp_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", tmp_file.c_str(), strerror(errno));
    return RC::IOERR;  // 创建索引中途出错，要做还原操作
  }
  if (new_table_meta.serialize(fs) < 0) {
    LOG_ERROR("Failed to dump new table meta to file: %s. sys err=%d:%s", tmp_file.c_str(), errno, strerror(errno));
    return RC::IOERR;
  }
  fs.close();

  // 覆盖原始元数据文�?
  std::string meta_file = table_meta_file(base_dir_.c_str(), name());
  int ret = rename(tmp_file.c_str(), meta_file.c_str());
  if (ret != 0) {
    LOG_ERROR("Failed to rename tmp meta file (%s) to normal meta file (%s) while creating index (%s) on table (%s). "
              "system error=%d:%s",
        tmp_file.c_str(),
        meta_file.c_str(),
        index_name,
        name(),
        errno,
        strerror(errno));
    return RC::IOERR;
  }

  table_meta_.swap(new_table_meta);

  LOG_INFO("Successfully added a new index (%s) on the table (%s)", index_name, name());

  return rc;
}

RC Table::create_multi_index(Trx *trx, const char *index_name, char *attributes[], int attr_num, int is_unique){
  if (common::is_blank(index_name)) {
    LOG_INFO("Invalid input arguments, table name is %s, index_name is blank", name());
    return RC::INVALID_ARGUMENT;
  }
  for (int i = 0; i < attr_num; ++i) {
    if (common::is_blank(attributes[i])){
      LOG_INFO("Invalid input arguments, table name is %s, attribute_name is blank", name());
      return RC::INVALID_ARGUMENT;
    }
  }
  if (table_meta_.index(index_name) != nullptr) {
    LOG_INFO("Invalid input arguments, table name is %s, index %s exist", name(), index_name);
    return RC::SCHEMA_INDEX_EXIST;
  }
  std::vector<const char*> fields;
  for (int i=0; i<attr_num; ++i){
    fields.push_back(attributes[i]);
  }
  if(table_meta_.find_index_by_fields(fields)) {
    LOG_INFO("Invalid input arguments, table name is %s, these attr already have index", name());
    return RC::SCHEMA_INDEX_EXIST;
  }
  std::vector<FieldMeta> field_metas;
  for (int i = 0; i < attr_num; ++i) {
    const FieldMeta *field_meta = table_meta_.field(attributes[i]);
    if (!field_meta) {
      LOG_INFO("Invalid input arguments, there is no field of %s in table:%s.", attributes[i], name());
      return RC::SCHEMA_FIELD_MISSING;
    }
    field_metas.push_back(*field_meta);
  }
  
  IndexMeta new_index_meta;
  RC rc = new_index_meta.init(index_name, field_metas, is_unique == 1);
  if (rc != RC::SUCCESS) {
    LOG_INFO("Failed to init IndexMeta in table:%s, index_name:%s", name(), index_name);
    return rc;
  }

  // 创建索引相关数据
  BplusTreeIndex *index = new BplusTreeIndex();
  std::string index_file = table_index_file(base_dir_.c_str(), name(), index_name);
  rc = index->create(index_file.c_str(), new_index_meta, field_metas);
  if (rc != RC::SUCCESS) {
    delete index;
    LOG_ERROR("Failed to create bplus tree index. file name=%s, rc=%d:%s", index_file.c_str(), rc, strrc(rc));
    return rc;
  }

  // 遍历当前的所有数据，插入这个索引
  // TODO: 唯一索引实现，发现表的索引列有重复数据时，失�?
  IndexInserter index_inserter(index);
  rc = scan_record(trx, nullptr, -1, &index_inserter, insert_index_record_reader_adapter);
  if (rc != RC::SUCCESS) {
    // rollback
    delete index;
    LOG_ERROR("Failed to insert index to all records. table=%s, rc=%d:%s", name(), rc, strrc(rc));
    return rc;
  }
  indexes_.push_back(index);

  TableMeta new_table_meta(table_meta_);
  rc = new_table_meta.add_index(new_index_meta);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to add index (%s) on table (%s). error=%d:%s", index_name, name(), rc, strrc(rc));
    return rc;
  }

  // 创建元数据临时文�?
  std::string tmp_file = table_meta_file(base_dir_.c_str(), name()) + ".tmp";
  std::fstream fs;
  fs.open(tmp_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", tmp_file.c_str(), strerror(errno));
    return RC::IOERR;  // 创建索引中途出错，要做还原操作
  }
  if (new_table_meta.serialize(fs) < 0) {
    LOG_ERROR("Failed to dump new table meta to file: %s. sys err=%d:%s", tmp_file.c_str(), errno, strerror(errno));
    return RC::IOERR;
  }
  fs.close();

  // 覆盖原始元数据文�?
  std::string meta_file = table_meta_file(base_dir_.c_str(), name());
  int ret = rename(tmp_file.c_str(), meta_file.c_str());
  if (ret != 0) {
    LOG_ERROR("Failed to rename tmp meta file (%s) to normal meta file (%s) while creating index (%s) on table (%s). "
              "system error=%d:%s",
        tmp_file.c_str(),
        meta_file.c_str(),
        index_name,
        name(),
        errno,
        strerror(errno));
    return RC::IOERR;
  }

  table_meta_.swap(new_table_meta);

  LOG_INFO("Successfully added a new index (%s) on the table (%s)", index_name, name());

  return rc;
}

// RC Table::update_record(Trx *trx, const char *attribute_name, const Value *value, int condition_num,
//     const Condition conditions[], int *updated_count)
// {
//   return RC::GENERIC_ERROR;
// }

RC Table::update_record(Trx *trx, Record *record){
  RC rc = RC::SUCCESS;

  if (trx != nullptr) {
    trx->init_trx_info(this, *record);
  }

  rc = record_handler_->update_record(record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Update record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return rc;
  }

  if (trx != nullptr) {
    rc = trx->update_record(this, record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to log operation(update) to trx");
      // RC rc2 = record_handler_->update_record(恢复原来)
      return RC::UNIMPLENMENT;
    }
  }
  
  rc = delete_entry_of_indexes(record->data(), record->rid(), true);
  rc = insert_entry_of_indexes(record->data(), record->rid());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Insert index for record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return RC::UNIMPLENMENT;
  }

  if (trx != nullptr) {
    // append clog record
    CLogRecord *clog_record = nullptr;
    rc = clog_manager_->clog_gen_record(CLogType::REDO_UPDATE, trx->get_current_id(), clog_record, name(), 0, record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create a clog record. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
    rc = clog_manager_->clog_append_record(clog_record);
    if (rc != RC::SUCCESS) {
      return rc;
    }  
  }
   
  return rc;
}

RC Table::recover_update_record(Record *record) {
  RC rc = RC::SUCCESS;

  rc = record_handler_->recover_update_record(record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Recover update record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return rc;
  }

  return rc;
}


class RecordDeleter {
public:
  RecordDeleter(Table &table, Trx *trx) : table_(table), trx_(trx)
  {}

  RC delete_record(Record *record)
  {
    RC rc = RC::SUCCESS;
    rc = table_.delete_record(trx_, record);
    if (rc == RC::SUCCESS) {
      deleted_count_++;
    }
    return rc;
  }

  int deleted_count() const
  {
    return deleted_count_;
  }

private:
  Table &table_;
  Trx *trx_;
  int deleted_count_ = 0;
};

static RC record_reader_delete_adapter(Record *record, void *context)
{
  RecordDeleter &record_deleter = *(RecordDeleter *)context;
  return record_deleter.delete_record(record);
}

RC Table::delete_record(Trx *trx, ConditionFilter *filter, int *deleted_count)
{
  RecordDeleter deleter(*this, trx);
  RC rc = scan_record(trx, filter, -1, &deleter, record_reader_delete_adapter);
  if (deleted_count != nullptr) {
    *deleted_count = deleter.deleted_count();
  }
  return rc;
}

RC Table::delete_record(Trx *trx, Record *record)
{
  RC rc = RC::SUCCESS;
  
  rc = delete_entry_of_indexes(record->data(), record->rid(), false);  // 重复代码 refer to commit_delete
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete indexes of record (rid=%d.%d). rc=%d:%s",
                record->rid().page_num, record->rid().slot_num, rc, strrc(rc));
    return rc;
  } 
  
  rc = record_handler_->delete_record(&record->rid());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete record (rid=%d.%d). rc=%d:%s",
                record->rid().page_num, record->rid().slot_num, rc, strrc(rc));
    return rc;
  }

  if (trx != nullptr) {
    rc = trx->delete_record(this, record);
    
    CLogRecord *clog_record = nullptr;
    rc = clog_manager_->clog_gen_record(CLogType::REDO_DELETE, trx->get_current_id(), clog_record, name(), 0, record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to create a clog record. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
    rc = clog_manager_->clog_append_record(clog_record);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }

  return rc;
}

RC Table::recover_delete_record(Record *record)
{
  RC rc = RC::SUCCESS;
  rc = record_handler_->delete_record(&record->rid());
  
  return rc;
}

RC Table::commit_delete(Trx *trx, const RID &rid)
{
  RC rc = RC::SUCCESS;
  Record record;
  rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  rc = delete_entry_of_indexes(record.data(), record.rid(), false);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete indexes of record(rid=%d.%d). rc=%d:%s",
        rid.page_num, rid.slot_num, rc, strrc(rc));  // panic?
  }

  rc = record_handler_->delete_record(&rid);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return rc;
}

RC Table::rollback_delete(Trx *trx, const RID &rid)
{
  RC rc = RC::SUCCESS;
  Record record;
  rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return trx->rollback_delete(this, record);  // update record in place
}

RC Table::rollback_update(Trx *trx, const RID &rid) {
  RC rc = RC::SUCCESS;
  Record record;
  rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  return trx->rollback_update(this, record);
}

RC Table::insert_entry_of_indexes(const char *record, const RID &rid)
{
  RC rc = RC::SUCCESS;
  for (Index *index : indexes_) {
    rc = index->insert_entry(record, &rid);
    if (rc != RC::SUCCESS) {
      break;
    }
  }
  return rc;
}

RC Table::delete_entry_of_indexes(const char *record, const RID &rid, bool error_on_not_exists)
{
  RC rc = RC::SUCCESS;
  for (Index *index : indexes_) {
    rc = index->delete_entry(record, &rid);
    if (rc != RC::SUCCESS) {
      if (rc != RC::RECORD_INVALID_KEY || !error_on_not_exists) {
        break;
      }
    }
  }
  return rc;
}

Index *Table::find_index(const char *index_name) const
{
  for (Index *index : indexes_) {
    if (0 == strcmp(index->index_meta().name(), index_name)) {
      return index;
    }
  }
  return nullptr;
}

// Index *Table::find_index_by_field(const char *field_name) const
// {
//   const TableMeta &table_meta = this->table_meta();
//   const IndexMeta *index_meta = table_meta.find_index_by_field(field_name);
//   if (index_meta != nullptr) {
//     return this->find_index(index_meta->name());
//   }
//   return nullptr;
// }

Index *Table::find_index_by_fields(std::vector<const char*> &field_names) const
{
  const TableMeta &table_meta = this->table_meta();
  const IndexMeta *index_meta = table_meta.find_index_by_fields(field_names);
  if (index_meta != nullptr) {
    return this->find_index(index_meta->name());
  }
  return nullptr;
}

// Index *Table::find_index_by_fields(char *field_names[], int fields_num) const
// {
//   const TableMeta &table_meta = this->table_meta();
//   const IndexMeta *index_meta = table_meta.find_index_by_fields(field_names, fields_num);
//   if (index_meta != nullptr) {
//     return this->find_index(index_meta->name());
//   }
//   return nullptr;
// }

IndexScanner *Table::find_index_for_scan(const DefaultConditionFilter &filter)
{
  const ConDesc *field_cond_desc = nullptr;
  const ConDesc *value_cond_desc = nullptr;
  if (filter.left().is_attr && !filter.right().is_attr) {
    field_cond_desc = &filter.left();
    value_cond_desc = &filter.right();
  } else if (filter.right().is_attr && !filter.left().is_attr) {
    field_cond_desc = &filter.right();
    value_cond_desc = &filter.left();
  }
  if (field_cond_desc == nullptr || value_cond_desc == nullptr) {
    return nullptr;
  }

  const FieldMeta *field_meta = table_meta_.find_field_by_offset(field_cond_desc->attr_offset);
  if (nullptr == field_meta) {
    LOG_PANIC("Cannot find field by offset %d. table=%s", field_cond_desc->attr_offset, name());
    return nullptr;
  }

  std::vector<const char*> fields;
  fields.push_back(field_meta->name());
  const IndexMeta *index_meta = table_meta_.find_index_by_fields(fields);
  if (nullptr == index_meta) {
    return nullptr;
  }

  Index *index = find_index(index_meta->name());
  if (nullptr == index) {
    return nullptr;
  }

  const char *left_key = nullptr;
  const char *right_key = nullptr;
  int left_len = 4;
  int right_len = 4;
  bool left_inclusive = false;
  bool right_inclusive = false;
  switch (filter.comp_op()) {
  case EQUAL_TO: {
    left_key = (const char *)value_cond_desc->value;
    right_key = (const char *)value_cond_desc->value;
    left_inclusive = true;
    right_inclusive = true;
  }
    break;
  case LESS_EQUAL: {
    right_key = (const char *)value_cond_desc->value;
    right_inclusive = true;
  }
    break;
  case GREAT_EQUAL: {
    left_key = (const char *)value_cond_desc->value;
    left_inclusive = true;
  }
    break;
  case LESS_THAN: {
    right_key = (const char *)value_cond_desc->value;
    right_inclusive = false;
  }
    break;
  case GREAT_THAN: {
    left_key = (const char *)value_cond_desc->value;
    left_inclusive = false;
  }
    break;
  case LIKE: {
    left_key = (const char *)value_cond_desc->value;
    right_key = (const char *)value_cond_desc->value;
    left_inclusive = true;
    right_inclusive = true;
  }
    break;
  default: {
    return nullptr;
  }
  }

  if (filter.attr_type() == CHARS) {
    left_len = left_key != nullptr ? strlen(left_key) : 0;
    right_len = right_key != nullptr ? strlen(right_key) : 0;
  }
  return index->create_scanner(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
}

IndexScanner *Table::find_index_for_scan(const ConditionFilter *filter)
{
  if (nullptr == filter) {
    return nullptr;
  }

  // remove dynamic_cast
  const DefaultConditionFilter *default_condition_filter = dynamic_cast<const DefaultConditionFilter *>(filter);
  if (default_condition_filter != nullptr) {
    return find_index_for_scan(*default_condition_filter);
  }

  const CompositeConditionFilter *composite_condition_filter = dynamic_cast<const CompositeConditionFilter *>(filter);
  if (composite_condition_filter != nullptr) {
    int filter_num = composite_condition_filter->filter_num();
    for (int i = 0; i < filter_num; i++) {
      IndexScanner *scanner = find_index_for_scan(&composite_condition_filter->filter(i));
      if (scanner != nullptr) {
        return scanner;  // 可以找到一个最优的，比如比较符号是=
      }
    }
  }
  return nullptr;
}

RC Table::sync()
{
  RC rc = RC::SUCCESS;
  for (Index *index : indexes_) {
    rc = index->sync();
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to flush index's pages. table=%s, index=%s, rc=%d:%s",
          name(), index->index_meta().name(), rc, strrc(rc));
      return rc;
    }
  }
  LOG_INFO("Sync table over. table=%s", name());
  return rc;
}

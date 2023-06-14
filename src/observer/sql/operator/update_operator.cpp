#include "sql/operator/update_operator.h"
#include "common/log/log.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/update_stmt.h"
#include "util/util.h"
#include <cstring>

RC UpdateOperator::open() {
  if (children_.size() != 1) {
    LOG_WARN("delete operator must has 1 child");
    return RC::INTERNAL;
  }

  Operator *child = children_[0];
  RC rc = child->open();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  Table *table = update_stmt_->table();
  while (RC::SUCCESS == (rc = child->next())) {
    Tuple *tuple = child->current_tuple();
      if (nullptr == tuple) {
        LOG_WARN("failed to get current record: %s", strrc(rc));
        return rc;
      }
    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record &record = row_tuple->record();

    const FieldMeta *sys_field_nullable = table->table_meta().field(1);
    TupleCell null_cell;
    rc = tuple->cell_at(1, null_cell);
    char *nullableBitmap_data = new char[sys_field_nullable->len()];
    memcpy(nullableBitmap_data, null_cell.data(), sys_field_nullable->len());

    char *record_data = new char[table->table_meta().record_size()];
    memset(record_data, 0, table->table_meta().record_size());
    for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); ++i) {
      const FieldMeta *field = table->table_meta().field(i);
      TupleCell tuple_cell;
      rc = row_tuple->cell_at(i, tuple_cell);
      if(rc != RC::SUCCESS) {
        LOG_WARN("failed to get tuple cell: %s", strrc(rc));
        return rc;
      }

      // 便利这一行每一个cell，看看需不需要更新
      int index = -1;
      for (int j = 0; j < update_stmt_->col_val_size(); ++j) {
        if (strcmp(update_stmt_->field(j).field_name(), field->name()) == 0) {
          index = j;
          break;
        }
      }
      if (index == -1) {
          size_t copy_len = field->len();
          if (field->type() == CHARS) {
            const size_t data_len = strlen((const char *)tuple_cell.data());
            if(copy_len > data_len) {
              copy_len = data_len + 1;
            }
          }
          memcpy(record_data + field->offset(), tuple_cell.data(), copy_len);
      }
      else {
      // col_val中第index个要被更新到当前cell
        Value update_value = update_stmt_->value(index);
        if(update_value.type != NULLS && update_value.type != field->type()
            && !(update_value.type == CHARS && field->type() == TEXTS)) {

          if (field->type() == INTS && update_value.type == CHARS) { // TODO try to convert the value type to field type
            char* val_chars = (char*)update_stmt_->value(index).data;
            int* val_ints = chars2ints(val_chars);
            Value value2;
            value2.type = INTS;
            value2.data = val_ints;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if (field->type() == CHARS && update_value.type == INTS) {
            int* val_ints = (int*)update_stmt_->value(index).data;
            char* val_chars = ints2chars(val_ints);
            Value value2;
            value2.type = CHARS;
            value2.data = val_chars;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if (field->type() == FLOATS && update_value.type == CHARS) {
            char* val_chars = (char*)update_stmt_->value(index).data;
            float* val_floats = chars2floats(val_chars);
            Value value2;
            value2.type = FLOATS;
            value2.data = val_floats;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if (field->type() == CHARS && update_value.type == FLOATS) {
            float* val_floats = (float*)update_stmt_->value(index).data;
            char* val_chars = floats2chars(val_floats);
            Value value2;
            value2.type = CHARS;
            value2.data = val_chars;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if (field->type() == FLOATS && update_value.type == INTS) {
            int* val_ints = (int*)update_stmt_->value(index).data;
            float* val_floats = ints2floats(val_ints);
            Value value2;
            value2.type = FLOATS;
            value2.data = val_floats;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if (field->type() == INTS && update_value.type == FLOATS) {
            float* val_floats = (float*)update_stmt_->value(index).data;
            int* val_ints = floats2ints(val_floats);
            Value value2;
            value2.type = INTS;
            value2.data = val_ints;
            update_stmt_->set_value(index, value2);
            // TBD: 释放内存
          }
          else if(field->type() == TEXTS && update_value.type == CHARS) {
            Value value2;
            value2.type = TEXTS;
            char *texts = new char[4097];
            memset(texts, 0, 4097);
            memcpy(texts, update_stmt_->value(index).data, strlen((char*)update_stmt_->value(index).data));
            value2.data = texts;
            update_stmt_->set_value(index, value2);
          }
          else if (field->type() != update_value.type) { 
            LOG_WARN("update schema field type mismatch: %s", strrc(rc));
            return RC::SCHEMA_FIELD_TYPE_MISMATCH;
          }

          update_value = update_stmt_->value(index);

          // LOG_WARN("update schema field type mismatch: %s", strrc(rc));
          // return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
        size_t copy_len = field->len();
        if (update_value.type != NULLS && field->type() == CHARS) {
          const size_t data_len = strlen((const char *)update_value.data);
          if(copy_len > data_len) {
            copy_len = data_len + 1;
          }
        }
        if (update_value.type == NULLS) {
          if (field->nullable() == false) {
            LOG_WARN("updating a non nullable field with null!");
            return RC::SCHEMA_FIELD_TYPE_MISMATCH;
          }
          uint8_t mask = (0x01 << (7 - (i-table->table_meta().sys_field_num()) % 8));
          nullableBitmap_data[(i-table->table_meta().sys_field_num()) / 8] |= mask;
          char *null_data = new char[field->len()];
          memset(null_data, 0, field->len());
          memcpy(record_data + field->offset(), null_data, field->len());
          delete []null_data;
        }
        else {
          if((nullableBitmap_data[(i-table->table_meta().sys_field_num())/BYTE_BIT] & (0x01 << (BYTE_MOVE_BIT-(i-table->table_meta().sys_field_num())%BYTE_BIT))) != 0) {
            uint8_t mask = (0x01 << (7 - (i-table->table_meta().sys_field_num()) % 8));
            mask = ~mask;
            nullableBitmap_data[(i-table->table_meta().sys_field_num()) / 8] &= mask;
          }
          memcpy(record_data + field->offset(), update_value.data, copy_len);
        }
      }
    }

    memcpy(record_data + sys_field_nullable->offset(), nullableBitmap_data, sys_field_nullable->len());
    rc = table->delete_record(trx_, &record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record while updating: %s", strrc(rc));
      return rc;
    } 
    
    Record new_record;
    new_record.set_rid(record.rid());
    new_record.set_data(record_data);
    // rc = table->update_record(trx_, &record);

    rc = table->insert_record(trx_, &new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record while updating: %s", strrc(rc));
      rc = table->insert_record(trx_, &record);
      if(rc != RC::SUCCESS) {
        LOG_WARN("Failed to restore delete");
        return rc;
      }
      return RC::INTERNAL;
    } 
    
    delete[] record_data;
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record: %s", strrc(rc));
      return rc;
    } 
  }
  return RC::SUCCESS;
}

RC UpdateOperator::next() {
  return RC::RECORD_EOF;
}

RC UpdateOperator::close() {
  children_[0]->close();
  return RC::SUCCESS;
}
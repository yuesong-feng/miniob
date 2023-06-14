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
// Created by Meiyi & Longda on 2021/4/13.
//

#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iterator>

#include "event/sql_event.h"
#include "sql/executor/execute_funcs.h"
#include "sql/expr/expression.h"
#include "sql/expr/res_tuple.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "storage/common/field.h"
#include "storage/common/table.h"

using namespace common;

//RC create_selection_executor(
//   Trx *trx, const Selects &selects, const char *db, const char *table_name, SelectExeNode &select_node);

//! Constructor
ExecuteStage::ExecuteStage(const char *tag) : Stage(tag)
{}

//! Destructor
ExecuteStage::~ExecuteStage()
{}

//! Parse properties, instantiate a stage object
Stage *ExecuteStage::make_stage(const std::string &tag)
{
  ExecuteStage *stage = new (std::nothrow) ExecuteStage(tag.c_str());
  if (stage == nullptr) {
    LOG_ERROR("new ExecuteStage failed");
    return nullptr;
  }
  stage->set_properties();
  return stage;
}

//! Set properties for this object set in stage specific properties
bool ExecuteStage::set_properties()
{
  //  std::string stageNameStr(stageName);
  //  std::map<std::string, std::string> section = theGlobalProperties()->get(
  //    stageNameStr);
  //
  //  std::map<std::string, std::string>::iterator it;
  //
  //  std::string key;

  return true;
}

//! Initialize stage params and validate outputs
bool ExecuteStage::initialize()
{
  LOG_TRACE("Enter");

  std::list<Stage *>::iterator stgp = next_stage_list_.begin();
  default_storage_stage_ = *(stgp++);
  mem_storage_stage_ = *(stgp++);

  LOG_TRACE("Exit");
  return true;
}

//! Cleanup after disconnection
void ExecuteStage::cleanup()
{
  LOG_TRACE("Enter");

  LOG_TRACE("Exit");
}

void ExecuteStage::handle_event(StageEvent *event)
{
  LOG_TRACE("Enter\n");

  handle_request(event);

  LOG_TRACE("Exit\n");
  return;
}

void ExecuteStage::callback_event(StageEvent *event, CallbackContext *context)
{
  LOG_TRACE("Enter\n");

  // here finish read all data from disk or network, but do nothing here.

  LOG_TRACE("Exit\n");
  return;
}

void ExecuteStage::handle_request(common::StageEvent *event)
{
  SQLStageEvent *sql_event = static_cast<SQLStageEvent *>(event);
  SessionEvent *session_event = sql_event->session_event();
  Stmt *stmt = sql_event->stmt();
  Session *session = session_event->session();
  Query *sql = sql_event->query();

  if (stmt != nullptr) {
    switch (stmt->type()) {
    case StmtType::SELECT: {
      std::vector<ResTuple> ret;
      do_select(sql_event, ret, true);
    } break;
    case StmtType::INSERT: {
      do_insert(sql_event);
    } break;
    case StmtType::UPDATE: {
      do_update(sql_event);
    } break;
    case StmtType::DELETE: {
      do_delete(sql_event);
    } break;
    default: {
      LOG_WARN("should not happen. please implenment");
    } break;
    }
  } else {
    switch (sql->flag) {
    case SCF_HELP: {
      do_help(sql_event);
    } break;
    case SCF_CREATE_TABLE: {
      do_create_table(sql_event);
    } break;
    case SCF_CREATE_INDEX: {
      do_create_index(sql_event);
    } break;
    case SCF_SHOW_INDEX: {
      do_show_index(sql_event);
    } break;
    case SCF_SHOW_TABLES: {
      do_show_tables(sql_event);
    } break;
    case SCF_DESC_TABLE: {
      do_desc_table(sql_event);
    } break;
    case SCF_DROP_TABLE: {
      do_drop_table(sql_event);
    } break;
    case SCF_DROP_INDEX:
    case SCF_LOAD_DATA: {
      default_storage_stage_->handle_event(event);
    } break;
    case SCF_SYNC: {
      /*
      RC rc = DefaultHandler::get_default().sync();
      session_event->set_response(strrc(rc));
      */
    } break;
    case SCF_BEGIN: {
      do_begin(sql_event);
      /*
      session_event->set_response("SUCCESS\n");
      */
    } break;
    case SCF_COMMIT: {
      do_commit(sql_event);
      /*
      Trx *trx = session->current_trx();
      RC rc = trx->commit();
      session->set_trx_multi_operation_mode(false);
      session_event->set_response(strrc(rc));
      */
    } break;
    case SCF_CLOG_SYNC: {
      do_clog_sync(sql_event);
    }
    case SCF_ROLLBACK: {
      Trx *trx = session_event->get_client()->session->current_trx();
      RC rc = trx->rollback();
      session->set_trx_multi_operation_mode(false);
      session_event->set_response(strrc(rc));
    } break;
    case SCF_EXIT: {
      // do nothing
      const char *response = "Unsupported\n";
      session_event->set_response(response);
    } break;
    default: {
      LOG_ERROR("Unsupported command=%d\n", sql->flag);
    }
    }
  }
}

IndexScanOperator *try_to_create_index_scan_operator(FilterStmt *filter_stmt)
{
  const std::vector<FilterUnit *> &filter_units = filter_stmt->filter_units();
  if (filter_units.empty() ) {
    return nullptr;
  }

  // 在所有过滤条件中，找到字段与值做比较的条件，然后判断字段是否可以使用索引
  // 如果是多列索引，这里的处理需要更复杂。
  // 这里的查找规则是比较简单的，就是尽量找到使用相等比较的索引
  // 如果没有就找范围比较的，但是直接排除不等比较的索引查询. (你知道为什么?)

  // TODO: 增加多列索引的逻辑
  const FilterUnit *better_filter = nullptr;
  for (const FilterUnit * filter_unit : filter_units) {
    if (filter_unit->comp() == NOT_EQUAL) {
      continue;
    }

    Expression *left = filter_unit->left();
    Expression *right = filter_unit->right();
    if (left->type() == ExprType::FIELD && right->type() == ExprType::VALUE) {
    } else if (left->type() == ExprType::VALUE && right->type() == ExprType::FIELD) {
      std::swap(left, right);
    } else if(left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
      continue;
    }
    FieldExpr &left_field_expr = *(FieldExpr *)left;
    const Field &field = left_field_expr.field();
    const Table *table = field.table();
    std::vector<const char*> fields;
    fields.push_back(field.field_name());
    Index *index = table->find_index_by_fields(fields);
    if (index != nullptr) {
      if (better_filter == nullptr) {
        better_filter = filter_unit;
      } else if (filter_unit->comp() == EQUAL_TO) {
        better_filter = filter_unit;
    	break;
      }
    }
  }

  if (better_filter == nullptr) {
    return nullptr;
  }

  Expression *left = better_filter->left();
  Expression *right = better_filter->right();
  CompOp comp = better_filter->comp();
  if (left->type() == ExprType::VALUE && right->type() == ExprType::FIELD) {
    std::swap(left, right);
    switch (comp) {
    case EQUAL_TO:    { comp = EQUAL_TO; }    break;
    case LESS_EQUAL:  { comp = GREAT_THAN; }  break;
    case NOT_EQUAL:   { comp = NOT_EQUAL; }   break;
    case LESS_THAN:   { comp = GREAT_EQUAL; } break;
    case GREAT_EQUAL: { comp = LESS_THAN; }   break;
    case GREAT_THAN:  { comp = LESS_EQUAL; }  break;
    case LIKE:        { comp = LIKE;    }     break;
    case NOT_LIKE:    { comp = NOT_LIKE; }    break;
    case IS_NULL:          { comp = IS_NULL; }          break;
    case IS_NOT_NULL:      { comp = IS_NOT_NULL; }          break;
    default: {
    	LOG_WARN("should not happen");
    }
    }
  }


  FieldExpr &left_field_expr = *(FieldExpr *)left;
  const Field &field = left_field_expr.field();
  const Table *table = field.table();
  std::vector<const char*> fields;
  fields.push_back(field.field_name());
  Index *index = table->find_index_by_fields(fields);
  assert(index != nullptr);

  ValueExpr &right_value_expr = *(ValueExpr *)right;
  TupleCell value;
  right_value_expr.get_tuple_cell(value);
  if (value.attr_type() == NULLS) {
    value.set_length(0);
  }
  else if (value.attr_type() != CHARS) {
    value.set_length(4);
  }

  const TupleCell *left_cell = nullptr;
  const TupleCell *right_cell = nullptr;
  bool left_inclusive = false;
  bool right_inclusive = false;

  switch (comp) {
  case EQUAL_TO: {
    left_cell = &value;
    right_cell = &value;
    left_inclusive = true;
    right_inclusive = true;
  } break;

  case LESS_EQUAL: {
    left_cell = nullptr;
    left_inclusive = false;
    right_cell = &value;
    right_inclusive = true;
  } break;

  case LESS_THAN: {
    left_cell = nullptr;
    left_inclusive = false;
    right_cell = &value;
    right_inclusive = false;
  } break;

  case GREAT_EQUAL: {
    left_cell = &value;
    left_inclusive = true;
    right_cell = nullptr;
    right_inclusive = false;
  } break;

  case GREAT_THAN: {
    left_cell = &value;
    left_inclusive = false;
    right_cell = nullptr;
    right_inclusive = false;
  } break;

  case LIKE: {
    left_cell = &value;
    right_cell = &value;
    left_inclusive = true;
    right_inclusive = true;
  } break;

  default: {
    LOG_WARN("should not happen. comp=%d", comp);
  } break;
  }

  std::vector<TupleCell> left_cells, right_cells;
  if(left_cell){
    left_cells.push_back(*left_cell);
  }
  if(right_cell){
    right_cells.push_back(*right_cell);
  }
  IndexScanOperator *oper = new IndexScanOperator(table, index,
       left_cells, left_inclusive, right_cells, right_inclusive);

  LOG_INFO("use index for scan: %s in table %s", index->index_meta().name(), table->name());
  return oper;
}

IndexScanOperator *create_index_scan_operator_by_index(FilterStmt *filter_stmt, Index *index)
{
  // 这个函数目前只在用于插入数据时检测是否违反unique index，filter_stmt中的属性即index对应的属性，且comp均为EQUAL_TO
  const std::vector<FilterUnit *> &filter_units = filter_stmt->filter_units();
  if (filter_units.empty() || index == nullptr) {
    LOG_WARN("filter_units or index should not be null");
    return nullptr;
  }

  std::vector<TupleCell> left_cells, right_cells;
  FieldExpr &left_field_expr = *(FieldExpr *)filter_units[0]->left();
  const Table *table = left_field_expr.field().table();
  for(auto filter_unit : filter_units) {
    Expression *left = filter_unit->left();
    Expression *right = filter_unit->right();

    ValueExpr &right_value_expr = *(ValueExpr *)right;
    TupleCell value;
    right_value_expr.get_tuple_cell(value);
    if(value.attr_type() != CHARS){
      value.set_length(4);
    }

    left_cells.push_back(value);
    right_cells.push_back(value);
  }

  bool left_inclusive = true;
  bool right_inclusive = true;

  IndexScanOperator *oper = new IndexScanOperator(table, index,
       left_cells, left_inclusive, right_cells, right_inclusive);

  LOG_INFO("use index for scan: %s in table %s", index->index_meta().name(), table->name());
  return oper;
}

RC ExecuteStage::do_select(SQLStageEvent *sql_event, std::vector<ResTuple> &ret_res, bool output)
{
  SelectStmt *select_stmt = (SelectStmt *)(sql_event->stmt());
  SessionEvent *session_event = sql_event->session_event();
  RC rc = RC::SUCCESS;
  if (select_stmt->tables().size() > 1) {
    return do_select_tables(sql_event);
  }

  Table *table = select_stmt->tables()[0];
  Operator *scan_oper = try_to_create_index_scan_operator(select_stmt->filter_stmt());
  if (nullptr == scan_oper) {
    scan_oper = new TableScanOperator(table);
  }

  DEFER([&] () {delete scan_oper;});

  // do sub-query
  for (int i = 0; i < select_stmt->subquery().size(); ++i) {
    SelectStmt *sub_stmt = (SelectStmt *)select_stmt->subquery()[i].subquery;
    SQLStageEvent *sub_event = new SQLStageEvent(sql_event->session_event(), sql_event->sql());
    sub_event->set_stmt(sub_stmt);
    sub_event->set_query(sql_event->query());
    // 计算sub-query，得到ret
    std::vector<ResTuple> ret;
    rc = do_select(sub_event, ret, false);
    if (rc != RC::SUCCESS) {
      session_event->set_response("FAILURE\n");
      return RC::GENERIC_ERROR;
    }
    // 把这个sub-query的结果转为Subset，ret只会有一列，多列则FAILURE
    if (ret.size() > 0 && ret[0].size() > 1) {
      session_event->set_response("FAILURE\n");
      return RC::GENERIC_ERROR;
    }
    Subset subset;
    subset.table_name = select_stmt->subquery()[i].table_name == nullptr ? nullptr : strdup(select_stmt->subquery()[i].table_name);
    subset.attribute_name = select_stmt->subquery()[i].attribute_name == nullptr ? nullptr : strdup(select_stmt->subquery()[i].attribute_name);
    subset.comp = select_stmt->subquery()[i].comp;
    subset.value_num = ret.size();
    if (subset.comp != IN_SUBQUERY && subset.comp != NOT_IN_SUBQUERY && subset.value_num > 1) {
      session_event->set_response("FAILURE\n");
      return RC::GENERIC_ERROR;
    }
    for (int j=0; j<subset.value_num; ++j) {
      Value value;
      value.type = ret[j].cell(0).attr_type();
      if (ret[j].cell(0).data() == nullptr) {
        value.data = nullptr;
      }
      else if (ret[j].cell(0).attr_type() == CHARS) {
        value.data = strdup(ret[j].cell(0).data());
      }
      else if (ret[j].cell(0).attr_type() == FLOATS){
        value.data = new float(*(float*)ret[j].cell(0).data());
      }
      else {
        value.data = new int(*(int*)ret[j].cell(0).data());
      }
      subset.values[j] = value;
    }
    select_stmt->add_subset(subset);
  }

  PredicateOperator pred_oper(select_stmt->filter_stmt());
  pred_oper.add_child(scan_oper);
  rc = pred_oper.open();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open operator");
    return rc;
  }
  std::stringstream ss;

  std::vector<ResTuple> results;
  while ((rc = pred_oper.next()) == RC::SUCCESS) {
    // get current record
    Tuple * tuple = pred_oper.current_tuple();    
    if (nullptr == tuple) {
      rc = RC::INTERNAL;
      LOG_WARN("failed to get current record. rc=%s", strrc(rc));
      break;
    }

    ResTuple res_tuple;
    std::vector<Field> fields;   // 当前table的fields
    for(int i=0; i<table->table_meta().field_num(); ++i){
      if(i == NULLABLE_COLUMN) continue;
      const FieldMeta *fieldMeta = table->table_meta().field(i);
      Field field(table, fieldMeta);
      fields.push_back(field);
    }
    res_tuple.set_fields(std::move(fields));
    TupleCell null_cell;
    rc = tuple->cell_at(NULLABLE_COLUMN, null_cell);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to fetch _null field of cell. rc=%s", strrc(rc));
        return RC::INTERNAL;
    }
    const char *null_data = null_cell.data();
    int sys_field_num = table->table_meta().sys_field_num();
    TupleCell cell;
    for (int i = 0; i < tuple->cell_num(); ++i) {
      if(i == NULLABLE_COLUMN) continue;
      rc = tuple->cell_at(i, cell);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to fetch field of cell. index=%d, rc=%s", i, strrc(rc));
        return RC::INTERNAL;
      }
      if (i >= sys_field_num && (null_data[(i-sys_field_num)/BYTE_BIT] & (0x01 << (BYTE_MOVE_BIT-(i-sys_field_num)%BYTE_BIT))) != 0) {
        cell.set_type(NULLS);
        cell.set_data("NULL");
      }
      res_tuple.add_cell(cell);
    }
    results.push_back(std::move(res_tuple));
  }
  if (rc != RC::RECORD_EOF) {
    LOG_WARN("something wrong while iterate operator. rc=%s", strrc(rc));
    pred_oper.close();
  } else {
    rc = pred_oper.close();
  }

  if (select_stmt->subsets().size() > 0) {
    rc = do_subset(results, select_stmt->subsets());
    if (rc != RC::SUCCESS) {
      LOG_WARN("subset failed!");
      return rc;
    }
  }

  // aggregation in expression
  bool have_agg_expression = expression_to_aggregation(select_stmt);

  std::vector<std::string> headers, results_headers;
  if (select_stmt->group_by_fields().size() > 0) {
    rc = do_group_by(results, select_stmt->agg_fields(), select_stmt->group_by_fields(), headers, results_headers, false, select_stmt->having_filter());
    if (rc != RC::SUCCESS) {
      LOG_WARN("Group by failed!");
      return rc;
    }
  }
  else if (select_stmt->agg_fields().size() > 0) {
    rc = do_aggregation(results, select_stmt->agg_fields(), headers, false);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Aggregation failed!");
      return rc;
    }
  }

  if (select_stmt->order_by_fields().size() > 0) {
    do_order_by(select_stmt->order_by_fields(), results);
  }

  std::vector<Field> query_headers;
  for (int i = 0; i < select_stmt->query_fields().size(); ++i) {
    const Field &field = select_stmt->query_fields()[i];
    query_headers.push_back(field);
  }
  
  if (output == true) {
    if (select_stmt->group_by_fields().size() > 0) {
      print_tuple_header(ss, headers);
      print_groupby_tuples(ss, results, headers, results_headers);
    } else if (select_stmt->agg_fields().size() > 0) {
      if (!have_agg_expression) {
        print_tuple_header(ss, headers);
        print_tuples(ss, results);
      }
      else {  // 这里假设所有query_headers都是expression，没有单独的agg
        print_tuple_header(ss, query_headers);
        print_agg_expression_tuples(ss, results, query_headers);
      }
    } else {
      print_tuple_header(ss, query_headers);
      print_tuples(ss, results, query_headers);
    }
    session_event->set_response(ss.str());
  }
  else {
    if (select_stmt->group_by_fields().size() > 0) {
      project_groupby_tuples(results, headers, results_headers);
    } else if (select_stmt->agg_fields().size() > 0) {
      // Nothing to do
    } else {
      project_tuples(results, query_headers);
    }
    ret_res = results;
  }
  return rc;
}

// 处理in，not in以及子查询等逻辑
RC ExecuteStage::do_subset(std::vector<ResTuple> &results, const std::vector<Subset> &subsets) {
  if (results.size() == 0) return RC::SUCCESS;

  for (Subset subset : subsets) {
    CompOp comp = subset.comp;
    int idx = -1;
    for (int i=0; i<results[0].size(); ++i) {
      if (strcmp(results[0].field(i).field_name(), subset.attribute_name) == 0) {
        idx = i;
        break;
      }
    }
    if (idx == -1) {
      LOG_WARN("subset.attribute_name not exit in results");
      return RC::INTERNAL;
    }
    std::vector<ResTuple> new_results;
    for (ResTuple resTuple : results) {
      if (comp==IN_SUBQUERY) {
        bool should_add = false;
        for (int i=0; i<subset.value_num; ++i) {
          if (subset.values[i].type!=NULLS && resTuple.cell(idx).attr_type()!=NULLS && 
          strcmp((char*)subset.values[i].data, resTuple.cell(idx).data()) == 0) {
            should_add = true;
            break;
          }
        }
        if (should_add) {
          new_results.push_back(resTuple);
        }
      }
      else if (comp==NOT_IN_SUBQUERY) {
        bool should_add = true;
        for (int i=0; i<subset.value_num; ++i) {
          if (subset.values[i].type==NULLS || resTuple.cell(idx).attr_type()==NULLS ||
           (subset.values[i].type!=NULLS && resTuple.cell(idx).attr_type()!=NULLS &&
          strcmp((char*)subset.values[i].data, resTuple.cell(idx).data()) == 0)) {
            should_add = false;
            break;
          }
        }
        if (should_add) {
          new_results.push_back(resTuple);
        }
      }
      else {
        if (subset.value_num == 0) {
          results.clear();
          return RC::SUCCESS;
        }
        if (subset.values[0].type == NULLS || resTuple.cell(idx).attr_type() == NULLS) {
          continue;
        }
        TupleCell tupleCell(subset.values[0].type, (char*)subset.values[0].data);
        if (subset.values[0].type == CHARS) {
          tupleCell.set_length(strlen((char*)subset.values[0].data));
        }
        else {
          tupleCell.set_length(4);
        }
        int ans = resTuple.cell(idx).compare(tupleCell);
        switch(comp) {
          case EQUAL_TO: {
            if (ans == 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
          case LESS_EQUAL: {
            if (ans <= 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
          case NOT_EQUAL: {
            if (ans != 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
          case LESS_THAN: {
            if (ans < 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
          case GREAT_EQUAL: {
            if (ans >= 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
          case GREAT_THAN: {
            if (ans > 0) {
              new_results.push_back(resTuple);
            }
            break;
          }
        }
      }
    }
    results = new_results;
  }
  return RC::SUCCESS;
}

// headers和agg_fields对应，results_headers和resutls对应
RC ExecuteStage::do_group_by(std::vector<ResTuple> &results, const std::vector<RelAttr> &agg_fields, const std::vector<Field> &group_by_fields, std::vector<std::string> &headers, std::vector<std::string> &results_headers, bool multi_table, FilterStmt *having_filter) {
  if (results.size() == 0) return RC::SUCCESS;
  for (auto agg_field : agg_fields) {
    std::string str = "";
    if (agg_field.agg_func == nullptr) {
      if (multi_table) {
        str += std::string(agg_field.relation_name) + ".";
      }
      str += std::string(agg_field.attribute_name);
    } else {
      str += agg_field.agg_func;
      if (strcmp(agg_field.attribute_name, "*") == 0) {
        str += "(*)";
      } else {
        str += "(";
        if (multi_table) {
          if (agg_field.relation_name == nullptr){
            LOG_WARN("agg_field.relation_name should not be null");
            return RC::INTERNAL;
          }
          str += std::string(agg_field.relation_name) + ".";
        }
        str += std::string(agg_field.attribute_name) + ")";
      }
    }
    headers.push_back(str);
  }
  
  // group by操作，根据group_by_fields里相同的一组，进行聚合操作
  // 会调用do_aggregation()
  // Step 1: 通过group_by_fields将results分组
  // Step 2: 每一组根据agg_fields调用do_aggregation
  // example: select id, avg(age) from t group by id;
  // 对每一组id相同的结果，调用do_aggregation()，把结果拼接

  // 所有group by属性在表中对应的索引
  std::vector<int> group_field_idx;
  
  for (auto group_by_field : group_by_fields) {
    int idx = -1;
    for (int j=0; j<results[0].fields().size(); ++j) {
      if(multi_table && strcmp(group_by_field.table_name(), results[0].field(j).table_name())!=0) {
        continue;
      }
      if (strcmp(group_by_field.field_name(), results[0].field(j).field_name())==0) {
        idx = j;
        break;
      }
    }
    if (idx==-1) {
      LOG_WARN("group_by_field not exit in table");
    }
    else {
      group_field_idx.push_back(idx);
    }
  }
  
  std::vector<RelAttr> agg_fields_with_func;  // 从agg_fields中筛选出带有func的field，用于do_aggregation的参数，不包括count(*)，这个可以直接在后面获取
  std::vector<int> agg_field_idx; // 所有带有func的field在表中对应的索引，不包括count(*)，这个可以直接在后面获取
  for (auto agg_field : agg_fields) {
    if (agg_field.agg_func != nullptr) {
      int idx = -1;
      agg_fields_with_func.push_back(agg_field);
      for (int j=0; j<results[0].fields().size(); ++j) {
        if (strcmp(agg_field.attribute_name, "*")==0 && strcmp(results[0].field(j).field_name(), "__trx")==0 
        && strcmp(agg_field.agg_func, "count")==0 && strcmp(results[0].field(j).meta()->agg_func(), "count")==0) {
          // 这种情况有可能表名为空，即使multi_table为true
          idx = j;
          break;
        }
        if(multi_table && strcmp(agg_field.relation_name, results[0].field(j).table_name())!=0) {
          continue;
        }
        if (strcmp(agg_field.attribute_name, results[0].field(j).field_name())==0) {
          idx = j;
          break;
        }
      }
      if (idx==-1) {
        LOG_WARN("group_by_field not exit in table");
      }
      else {
        agg_field_idx.push_back(idx);
      }
    }
  }

  // 将ResTuple对应的group by属性的值进行拼接得到string作为unordered_map的key
  std::unordered_map<std::string, std::vector<ResTuple>> GroupFields_2_SubResults;  // 将result中agg_fields_with_func属性组成的子表作为value
  std::unordered_map<std::string, ResTuple> GroupFields_2_GroupTuple;      // 将result中group by属性组成的ResTuple作为value
  for (auto result : results) {
    std::string str = "";
    ResTuple group_tuple;
    for (auto idx : group_field_idx) {
      str += std::string(result.cell(idx).data());
      group_tuple.add_field(result.field(idx));
      group_tuple.add_cell(result.cell(idx));
    }
    GroupFields_2_GroupTuple[str] = group_tuple;

    ResTuple agg_tuple;
    for (auto idx : agg_field_idx) {
      agg_tuple.add_field(result.field(idx));
      agg_tuple.add_cell(result.cell(idx));
    }
    GroupFields_2_SubResults[str].push_back(agg_tuple);
  }

  std::vector<ResTuple> ret;
  std::vector<std::string> agg_headers;
  std::vector<bool> init_results_headers(results[0].size()+1, false);
  for (std::unordered_map<std::string, std::vector<ResTuple>>::iterator iter=GroupFields_2_SubResults.begin(); iter!=GroupFields_2_SubResults.end(); ++iter) {
    std::string str = iter->first;
    std::vector<ResTuple> sub_results = iter->second;
    ResTuple group_tuple = GroupFields_2_GroupTuple[str];
    int *count_star_data = new int(sub_results.size());  // count(*)的值
    do_aggregation(sub_results, agg_fields_with_func, agg_headers, multi_table);
    ResTuple resTuple;
    for (int i=0; i<results[0].size(); ++i) {
      Field field = results[0].field(i);
      bool added = false;
      for (int j=0; j < group_tuple.size(); ++j) {
        if(multi_table && strcmp(group_tuple.field(j).table_name(), field.table_name())!=0) {
          continue;
        }
        if (strcmp(group_tuple.field(j).field_name(), field.field_name())==0) {
          resTuple.add_field(group_tuple.field(j));
          resTuple.add_cell(group_tuple.cell(j));
          added = true;
          if (!init_results_headers[i]) {
            std::string str = "";
            if (multi_table) {
              str += std::string(field.table_name()) + ".";
            }
            str += field.field_name();
            results_headers.push_back(str);
            init_results_headers[i] = true;
          }
          break;
        }
      }
      if (added) continue;
      for (int j=0; j < sub_results[0].size(); ++j) {
        if (strcmp(sub_results[0].field(j).field_name(), field.field_name())==0) {
          resTuple.add_field(sub_results[0].field(j));
          resTuple.add_cell(sub_results[0].cell(j));
          added = true;
          if (!init_results_headers[i]) {
            results_headers.push_back(agg_headers[j]);
            init_results_headers[i] = true;
          }
          break;
        }
      }
      if (!added) {
        // 如果results的某一列既不在group_by_fields中，又不在agg_fields中，那它也不会参与排序，所以可以忽略
      }
    }
    // 无论agg中是否有count(*)，都加上count(*)，用于having
    std::string count_star_field_meta_name = "*";
    FieldMeta *count_star_field_meta = new FieldMeta();
    count_star_field_meta->init(count_star_field_meta_name.c_str(), INTS, 0, 4, true, false);
    count_star_field_meta->set_agg_func("count");
    Field *count_star_field = new Field(nullptr, count_star_field_meta);
    TupleCell *count_star_cell = new TupleCell(count_star_field_meta, (char*)count_star_data);
    resTuple.add_field(*count_star_field);
    resTuple.add_cell(*count_star_cell);
    if (!init_results_headers[results[0].size()]) {
      results_headers.push_back("count(*)");
      init_results_headers[results[0].size()] = true;
    }
    ret.push_back(resTuple);
  }

  results = ret;

  // 根据having filter筛选ResTuple
  for (FilterUnit *unit : having_filter->filter_units()) {
    if(unit->left()->type() == ExprType::FIELD && unit->right()->type() == ExprType::VALUE) {
      filter_by_having(results, unit, true);
    }
  }

  return RC::SUCCESS;
}

RC ExecuteStage::do_order_by(const std::vector<Field> &order_by_fields, std::vector<ResTuple> &results) {
  for (int i = 0; i < order_by_fields.size(); ++i){
    const char *alias = order_by_fields[i].meta()->alias();
    if(strcmp(alias, "desc") != 0 && strcmp(alias, "asc") != 0){
      LOG_WARN("Order by alias mush be asc or desc");
      return RC::INTERNAL;
    }
  }

  // 注意order_by_fields是反的，最后一个优先级最高
  sort(results.begin(), results.end(), [=](ResTuple& resTuple1, ResTuple& resTuple2){
    for (int i = 0; i < order_by_fields.size(); ++i){
      const char *alias = order_by_fields[i].meta()->alias();
      const char *table_name = order_by_fields[i].table_name();
      const char *field_name = order_by_fields[i].field_name();
      for(int j=0; j<resTuple1.size(); ++j){
        if(strcmp(resTuple1.field(j).table_name(), table_name)==0 && strcmp(resTuple1.field(j).field_name(), field_name)==0){
          TupleCell tupleCell_1 = resTuple1.cell(j), tupleCell_2 = resTuple2.cell(j);
          // if (tupleCell_1.attr_type() == NULLS && tupleCell_2.attr_type() == NULLS) {
          //   break;
          // } else if (tupleCell_1.attr_type() == NULLS && tupleCell_2.attr_type() != NULLS) {
          //   return strcmp(alias, "asc") == 0;
          // } else if (tupleCell_1.attr_type() != NULLS && tupleCell_2.attr_type() == NULLS) {
          //   return strcmp(alias, "desc") == 0;
          // } else 
          if(tupleCell_1.compare(tupleCell_2) > 0){
            return strcmp(alias, "desc") == 0;
          } else if(tupleCell_1.compare(tupleCell_2) < 0){
            return strcmp(alias, "asc") == 0;
          } else{
            break;
          }
        }
      }
    }
    return true;
  });

  return RC::SUCCESS;
}

// 遍历agg_fields中，并从resutls中找到对应的列进行聚合，输出resutls的顺序与agg_fields的顺序一致
RC ExecuteStage::do_aggregation(std::vector<ResTuple> &results, const std::vector<RelAttr> &agg_fields, std::vector<std::string> &agg_headers, bool multi_table) {
  if (results.size() == 0) return RC::SUCCESS;
  for (auto agg_field : agg_fields) {
    std::string str = agg_field.agg_func;
    if (strcmp(agg_field.attribute_name, "*") == 0) {  // 表示*
      str += "(*)";
    } else {
      str += "(";
      if (multi_table) {
        if (agg_field.relation_name == nullptr){
          LOG_WARN("agg_field.relation_name should not be null");
          return RC::INTERNAL;
        }
        str += std::string(agg_field.relation_name) + ".";
      }
      str += std::string(agg_field.attribute_name) + ")";
    }
    // ret.field(i).meta()->set_alias(str);
    agg_headers.push_back(str);
  }
  ResTuple ret;
  for (auto agg_field : agg_fields) {
    const char* str = agg_field.agg_func;
    Field field;
    int idx = -1;
    for (int i=0; i<results[0].size(); ++i) {
      field = results[0].field(i);
      if (strcmp(field.field_name(), agg_field.attribute_name)==0 || 
      (strcmp(field.field_name(), "__trx")==0 && strcmp(agg_field.attribute_name, "*")==0)) {
        idx = i;
        break;
      }
    }
    if(idx==-1){
      LOG_WARN("agg_field not exit in results");
      return RC::INTERNAL;
    }
    TupleCell cell;
    field = results[0].field(idx);
    cell.set_length(field.meta()->len());
    cell.set_type(field.attr_type());
    if (strcmp(str, "count")==0) {
      int size = results.size();
      for (int j = 0; j < results.size(); ++j) {
        if(results[j].cell(idx).attr_type() == NULLS) {
          size--;
        }
      }
      if (cell.attr_type() == DATES || cell.attr_type() == FLOATS) {
        cell.set_type(INTS);
      }
      cell.set_data((char*)new int(size));
    } else if(strcmp(str, "sum")==0) {
      if(field.attr_type()==INTS){
        int sum = 0;
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if (results[j].cell(idx).attr_type() == NULLS) continue;
          sum += *(int*)results[j].cell(idx).data();
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new int(sum));
      }
      else if(field.attr_type()==FLOATS){
        float sum = 0;
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if (results[j].cell(idx).attr_type() == NULLS) continue;
          sum += *(float*)results[j].cell(idx).data();
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new float(sum));
      }
      else {
        LOG_WARN("can only calculate sum of int/float");
        return RC::INTERNAL;
      }
    } else if(strcmp(str, "max")==0) {
      if(field.attr_type()==INTS || field.attr_type()==DATES){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)>0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new int(*(int*)tmp.data()));
      }
      else if(field.attr_type()==FLOATS){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)>0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new int(*(int*)tmp.data()));
      }
      else if(field.attr_type()==CHARS){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)>0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else {
          char *data = new char[tmp.length()+1];
          memset(data, 0, sizeof(char)*(tmp.length() + 1));
          memcpy(data, tmp.data(), tmp.length());
          cell.set_data(data);
        }
      }
      else {
        LOG_WARN("can only calculate max of int/float/data");
        return RC::INTERNAL;
      }
    } else if(strcmp(str, "min")==0) {
      if(field.attr_type()==INTS || field.attr_type()==DATES){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)<0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new int(*(int*)tmp.data()));
      }
      else if(field.attr_type()==FLOATS){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)<0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else 
          cell.set_data((char*)new int(*(int*)tmp.data()));
      }
      else if(field.attr_type()==CHARS){
        TupleCell tmp = results[0].cell(idx);
        bool all_null = true;
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() != NULLS) all_null = false;
          if(results[j].cell(idx).compare(tmp)<0){
            tmp = results[j].cell(idx);
          }
        }
        if (all_null) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else {
          char *data = new char[tmp.length()+1];
          memset(data, 0, sizeof(char)*(tmp.length()+1));
          memcpy(data, tmp.data(), tmp.length());
          cell.set_data(data);
        }
      }
      else {
        LOG_WARN("can only calculate min of int/float/data");
        return RC::INTERNAL;
      }
    } else if(strcmp(str, "avg")==0) {
      if(field.attr_type()==INTS){
        float avg = 0;
        int size = results.size();
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() == NULLS) {
            size--;
            continue;
          }
          avg += *(int*)results[j].cell(idx).data();
        }
        if (size == 0 && results.size() != 0) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else {
          avg /= size;
          cell.set_type(FLOATS);
          cell.set_data((char*)new float(avg));
        }
      }
      else if(field.attr_type()==FLOATS){
        float avg = 0;
        int size = results.size();
        for(int j=0; j<results.size(); ++j){
          if (results[j].cell(idx).attr_type() == NULLS) {
            size--;
            continue;
          }
          avg += *(float*)results[j].cell(idx).data();
        }
        if (size == 0 && results.size() != 0) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else {
          avg /= size;
          cell.set_data((char*)new float(avg));
        }
      }
      else if(field.attr_type() == CHARS) {
        float avg = 0;
        int size = results.size();
        for(int j = 0; j < results.size(); ++j) {
          if (results[j].cell(idx).attr_type() == NULLS){
            size--;
            continue;
          }
          avg += *chars2floats(results[j].cell(idx).data());
        }
        if (size == 0 && results.size() != 0) {
          cell.set_type(CHARS);
          cell.set_data("NULL");
        } else {
          avg /= size;
          cell.set_type(FLOATS);
          cell.set_data((char*)new float(avg));
        }
      }
      else {
        LOG_WARN("can only calculate avg of int/float");
        return RC::INTERNAL;
      }
    }
    ret.add_field(results[0].field(idx));
    ret.add_cell(cell);
  }
  
  results.clear();
  results.push_back(std::move(ret));
  return RC::SUCCESS;
}

RC ExecuteStage::do_select_tables(SQLStageEvent *sql_event) {
  SelectStmt *select_stmt = (SelectStmt *)(sql_event->stmt());
  SessionEvent *session_event = sql_event->session_event();
  RC rc = RC::SUCCESS;

  std::stringstream ss;
  std::vector<std::vector<ResTuple>> results;
  std::vector<const char*> table_names;
  std::vector<FilterUnit*> expression_filter_units;
  for(Table *table : select_stmt->tables()) {
    // ss << table->name() << std::endl;
    // 获取当前table相关的filter_stmt，选择下推，中间结果更少
    FilterStmt *filter_stmt = new FilterStmt(); // TODO: 内存泄漏未解决
    for(FilterUnit *filter_unit : select_stmt->filter_stmt()->filter_units()) {
      Expression *left = filter_unit->left();
      Expression *right = filter_unit->right();
      if (left->type() == ExprType::VALUE && right->type() == ExprType::FIELD) {
        std::swap(left, right);
      } else if(left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
        // t1.id = t2.id
        continue;
      } else if(left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
        // value=value, eg: 1=1
        // TODO: 
        ValueExpr *left_expr = static_cast<ValueExpr *>(filter_unit->left());
        ValueExpr *right_expr = static_cast<ValueExpr *>(filter_unit->right());
        TupleCell left_tuple, right_tuple;
        left_expr->get_tuple_cell(left_tuple);
        right_expr->get_tuple_cell(right_tuple);
        if (left_tuple.attr_type() == EXPRESSION || right_tuple.attr_type() == EXPRESSION) {
          expression_filter_units.push_back(filter_unit);
          continue;
        }
        if ( left_tuple.compare(right_tuple) == 0 ) { // true 1=1
          // do nothing
        } else{ //false 1=2
          delete filter_stmt;
          filter_stmt = nullptr;
          filter_stmt = new FilterStmt();
          break;
        }
      }
      FieldExpr &left_field_expr = *(FieldExpr *)left;
      ValueExpr &right_value_expr = *(ValueExpr *)right;
      TupleCell right_tuple;
      right_value_expr.get_tuple_cell(right_tuple);
      if (right_tuple.attr_type() == EXPRESSION) {
        expression_filter_units.push_back(filter_unit);
        continue;
      }
      const Field &field = left_field_expr.field();
      if (strcmp(table->name(), field.table()->name()) == 0) {
        filter_stmt->add_unit(filter_unit);
      }
    }
    for(FilterStmt *fs : select_stmt->join_filters()) {
      for(FilterUnit *filter_unit : fs->filter_units()) {
        Expression *left = filter_unit->left();
        Expression *right = filter_unit->right();
        if (left->type() == ExprType::VALUE && right->type() == ExprType::FIELD) {
          std::swap(left, right);
        } else if(left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
          // t1.id = t2.id
          continue;
        } else if(left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
          // value=value, eg: 1=1
          // TODO: 
          continue;
        }
        FieldExpr &left_field_expr = *(FieldExpr *)left;
        const Field &field = left_field_expr.field();
        if (strcmp(table->name(), field.table()->name()) == 0) {
          filter_stmt->add_unit(filter_unit);
        }
      }
    }

    Operator *scan_oper = try_to_create_index_scan_operator(filter_stmt);
    if (nullptr == scan_oper) {
      scan_oper = new TableScanOperator(table);
    }
    // DEFER([&] () {delete scan_oper;});
    PredicateOperator pred_oper(filter_stmt);
    pred_oper.add_child(scan_oper);
    rc = pred_oper.open();
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to open operator");
      return rc;
    }
    
    std::vector<ResTuple> one_result;
    while ((rc = pred_oper.next()) == RC::SUCCESS) {
      // get current record
      Tuple * tuple = pred_oper.current_tuple();
      if (nullptr == tuple) {
        rc = RC::INTERNAL;
        LOG_WARN("failed to get current record. rc=%s", strrc(rc));
        break;
      }
      ResTuple res_tuple;
      // res_tuple.set_fields 设置为这个table的fields
      std::vector<Field> fields;   // 当前table的fields
      for(int i=table->table_meta().sys_field_num(); i<table->table_meta().field_num(); ++i){
        const FieldMeta *fieldMeta = table->table_meta().field(i);
        Field field(table, fieldMeta);
        fields.push_back(field);
      }
      res_tuple.set_fields(std::move(fields));
      TupleCell null_cell;
      tuple->cell_at(NULLABLE_COLUMN, null_cell);
      const char *null_data = null_cell.data();
      TupleCell cell;
      int sys_field_num = table->table_meta().sys_field_num();
      for (int i = sys_field_num; i < tuple->cell_num(); ++i) {
        rc = tuple->cell_at(i, cell);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to fetch field of cell. index=%d, rc=%s", i, strrc(rc));
          return RC::INTERNAL;
        }
        if ((null_data[(i-sys_field_num)/BYTE_BIT] & (0x01 << (BYTE_MOVE_BIT-(i-sys_field_num)%BYTE_BIT))) != 0) {
          cell.set_type(NULLS);
          cell.set_data("NULL");
        }
        res_tuple.add_cell(cell);
      }
      one_result.push_back(std::move(res_tuple));
    }

    if (rc != RC::RECORD_EOF) {
      LOG_WARN("something wrong while iterate operator. rc=%s", strrc(rc));
      pred_oper.close();
    } else {
      rc = pred_oper.close();
    }

    results.push_back(std::move(one_result));
    table_names.push_back(table->name());
  }

  std::vector<ResTuple> join_tuples;

  FilterStmt *where_filter = new FilterStmt();
  for (FilterUnit *filter_unit : select_stmt->filter_stmt()->filter_units()) {
    Expression *left = filter_unit->left();
    Expression *right = filter_unit->right();
    if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
      where_filter->add_unit(filter_unit);
    }
  }

  std::vector<FilterStmt *> join_filter;
  for(FilterStmt *fs : select_stmt->join_filters()) {
    FilterStmt *new_fs = new FilterStmt();
    for(FilterUnit *filter_unit : fs->filter_units()) {
      Expression *left = filter_unit->left();
      Expression *right = filter_unit->right();
      if (left->type() == ExprType::FIELD && right->type() == ExprType::FIELD) {
        new_fs->add_unit(filter_unit);
      } else if(left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
        new_fs->add_unit(filter_unit);
      }
    }
    join_filter.push_back(new_fs);
  }
  do_join(results, table_names, where_filter, join_filter, join_tuples);

  std::vector<std::string> headers, results_headers;
  if (select_stmt->group_by_fields().size() > 0) {
    rc = do_group_by(join_tuples, select_stmt->agg_fields(), select_stmt->group_by_fields(), headers, results_headers, true, select_stmt->having_filter());
    if (rc != RC::SUCCESS) {
      LOG_WARN("Group by failed!");
      return rc;
    }
  }

  if (select_stmt->order_by_fields().size() > 0) {
    do_order_by(select_stmt->order_by_fields(), join_tuples);
  }

  std::vector<Field> query_headers;
  for (int i = 0; i < select_stmt->query_fields().size(); ++i) {
    const Field &field = select_stmt->query_fields()[i];
    query_headers.push_back(field);
  }

  if (select_stmt->group_by_fields().size() > 0) {
    print_tuple_header(ss, headers);
    print_groupby_tuples(ss, join_tuples, headers, results_headers);
  }
  else {
    print_tuple_header_with_table(ss, query_headers);
    print_tuples(ss, join_tuples, query_headers, expression_filter_units);
  }

  session_event->set_response(ss.str());
  return rc;
}

RC ExecuteStage::do_join(std::vector<std::vector<ResTuple>> &results, std::vector<const char*> &table_names, FilterStmt *where_filter, std::vector<FilterStmt *> &join_filters, std::vector<ResTuple> &out_tuples){
  // results: 结果
  // filter: 条件
  // out_tuples: 输出的tuple
  // join_filter: 每一个join on的FilterStmt

  // 原来把join on所有条件都当where，不能处理1=1、1=2情况，需要细分为每一个join的FilterStmt
  // 对每一个FilterStmt，如果有1=2 false的情况，由于都是and，就清空，1=1 true则跳过，剩下的就可以当成一个整体FilterStmt
  FilterStmt *filter = new FilterStmt();
  for (FilterUnit *unit : where_filter->filter_units()) {
    filter->add_unit(unit);
  }
  for (FilterStmt *each_join_filter : join_filters) {
    std::vector<FilterUnit *> tmp;
    for (FilterUnit *unit : each_join_filter->filter_units()) {
      ValueExpr *left_expr = static_cast<ValueExpr *>(unit->left());
      ValueExpr *right_expr = static_cast<ValueExpr *>(unit->right());
      if (left_expr->type() == ExprType::VALUE && right_expr->type() == ExprType::VALUE) {
        TupleCell left_cell, right_cell;
        left_expr->get_tuple_cell(left_cell);
        right_expr->get_tuple_cell(right_cell);
        CompOp comp = unit->comp();
        const int compare = left_cell.compare(right_cell);
        bool filter_result = false;
        switch (comp) {
        case EQUAL_TO: {
          filter_result = (0 == compare); 
        } break;
        case LESS_EQUAL: {
          filter_result = (compare <= 0); 
        } break;
        case NOT_EQUAL: {
          filter_result = (compare != 0);
        } break;
        case LESS_THAN: {
          filter_result = (compare < 0);
        } break;
        case GREAT_EQUAL: {
          filter_result = (compare >= 0);
        } break;
        case GREAT_THAN: {
          filter_result = (compare > 0);
        } break;
        case LIKE: {
          filter_result = left_cell.like(right_cell);
        } break;
        case NOT_LIKE: {
          filter_result = !left_cell.like(right_cell);
        } break;
        default: {
          LOG_WARN("invalid compare type: %d", comp);
        } break;
        }
        if ( filter_result ) { // true
          // do nothing
        } else{ //false
          return RC::SUCCESS;
        }
      } else {
        tmp.push_back(unit);
      }
    }

    for (FilterUnit *unit : tmp) {
      filter->add_unit(unit);
    }
  }

  // Step1: 根据filter中两个表的连接信息提前过滤每个表
  for(FilterUnit *filter_unit : filter->filter_units()) {
    CompOp comp = filter_unit->comp();
    Expression *left = filter_unit->left();
    Expression *right = filter_unit->right();
    FieldExpr &left_field_expr = *(FieldExpr *)left;
    FieldExpr &right_field_expr = *(FieldExpr *)right;
    const Field &left_field = left_field_expr.field();
    const Field &right_field = right_field_expr.field();
    int left_index = -1, right_index = -1;
    for(int i=0; i<results.size(); ++i){
      if(strcmp(table_names[i], left_field.table_name())==0){
        left_index = i;
        break;
      }
    }
    if(left_index==-1){
      LOG_WARN("filter involves wrong table");
      return RC::INTERNAL;
    }
    for(int i=0; i<results.size(); ++i){
      if(strcmp(table_names[i], right_field.table_name())==0){
        right_index = i;
        break;
      }
    }
    if(right_index==-1){
      LOG_WARN("filter involves wrong table");
      return RC::INTERNAL;
    }
    filter_by_join_field(results[left_index], results[right_index], filter_unit);
  }

  // Step2: 从后往前join表，每次join都检查所有filter
  out_tuples = results.back();
  for (int i = results.size() - 2; i >= 0; --i) {
    out_tuples = two_table_cast(out_tuples, results[i], filter);
  }

  return RC::SUCCESS;
}

RC ExecuteStage::do_help(SQLStageEvent *sql_event)
{
  SessionEvent *session_event = sql_event->session_event();
  const char *response = "show tables;\n"
                         "desc `table name`;\n"
                         "create table `table name` (`column name` `column type`, ...);\n"
                         "create index `index name` on `table` (`column`);\n"
                         "insert into `table` values(`value1`,`value2`);\n"
                         "update `table` set column=value [where `column`=`value`];\n"
                         "delete from `table` [where `column`=`value`];\n"
                         "select [ * | `columns` ] from `table`;\n";
  session_event->set_response(response);
  return RC::SUCCESS;
}

RC ExecuteStage::do_create_table(SQLStageEvent *sql_event)
{
  const CreateTable &create_table = sql_event->query()->sstr.create_table;
  SessionEvent *session_event = sql_event->session_event();
  Db *db = session_event->session()->get_current_db();
  RC rc = db->create_table(create_table.relation_name,
			create_table.attribute_count, create_table.attributes);
  if (rc == RC::SUCCESS) {
    session_event->set_response("SUCCESS\n");
  } else {
    session_event->set_response("FAILURE\n");
  }
  return rc;
}
RC ExecuteStage::do_create_index(SQLStageEvent *sql_event)
{
  SessionEvent *session_event = sql_event->session_event();
  Db *db = session_event->session()->get_current_db();
  CreateIndex &create_index = sql_event->query()->sstr.create_index;
  for (int i = 0, j = create_index.attr_num - 1; i < j; i++,j--) 
    std::swap(create_index.attributes[i], create_index.attributes[j]);

  Table *table = db->find_table(create_index.relation_name);
  if (nullptr == table) {
    session_event->set_response("FAILURE\n");
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  RC rc = RC::INTERNAL;
  if (create_index.attr_num == 1) {
    rc = table->create_index(nullptr, create_index.index_name, create_index.attributes[0], create_index.is_unique);
  }
  else if(create_index.attr_num > 1) {
    rc = table->create_multi_index(nullptr, create_index.index_name, create_index.attributes, create_index.attr_num, create_index.is_unique);
  }
  sql_event->session_event()->set_response(rc == RC::SUCCESS ? "SUCCESS\n" : "FAILURE\n");
  return rc;
}

RC ExecuteStage::do_show_index(SQLStageEvent *sql_event){
  Query *query = sql_event->query();
  Db *db = sql_event->session_event()->session()->get_current_db();
  const char *table_name = query->sstr.show_index.relation_name;
  Table *table = db->find_table(table_name);
  if (table == nullptr) {
    LOG_WARN("Show index from a non-exist table %s", table_name);
    sql_event->session_event()->set_response("FAILURE\n");
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  std::stringstream ss;
  ss << "TABLE | NON_UNIQUE | KEY_NAME | SEQ_IN_INDEX | COLUMN_NAME" << std::endl;
  for(int i = 0; i < table->table_meta().index_num(); ++i) {
    const IndexMeta *index = table->table_meta().index(i);
    for (int i=0; i < index->fields_num(); ++i) {
      ss << table_name << " | "             // 表名
      << ((index->is_unique() == true) ? "0" : "1") << " | "            // 是否唯一索引
      << index->name() << " | "             // 索引名
      << i+1 << " | " << index->fields(i) << std::endl;   // 列在索引中的序号（针对多列索引） 和 索引所在列名
    }                  
  }
  
  sql_event->session_event()->set_response(ss.str().c_str());
  return RC::SUCCESS;
}

RC ExecuteStage::do_show_tables(SQLStageEvent *sql_event)
{
  SessionEvent *session_event = sql_event->session_event();
  Db *db = session_event->session()->get_current_db();
  std::vector<std::string> all_tables;
  db->all_tables(all_tables);
  if (all_tables.empty()) {
    session_event->set_response("No table\n");
  } else {
    std::stringstream ss;
    for (const auto &table : all_tables) {
      ss << table << std::endl;
    }
    session_event->set_response(ss.str().c_str());
  }
  return RC::SUCCESS;
}

RC ExecuteStage::do_desc_table(SQLStageEvent *sql_event)
{
  Query *query = sql_event->query();
  Db *db = sql_event->session_event()->session()->get_current_db();
  const char *table_name = query->sstr.desc_table.relation_name;
  Table *table = db->find_table(table_name);
  std::stringstream ss;
  if (table != nullptr) {
    table->table_meta().desc(ss);
  } else {
    ss << "No such table: " << table_name << std::endl;
  }
  sql_event->session_event()->set_response(ss.str().c_str());
  return RC::SUCCESS;
}

RC ExecuteStage::do_drop_table(SQLStageEvent *sql_event) {
  const DropTable &drop_table = sql_event->query()->sstr.drop_table;
  SessionEvent *session_event = sql_event->session_event();
  Db *db = session_event->session()->get_current_db();
  RC rc = db->drop_table(drop_table.relation_name);
  if (rc == RC::SUCCESS) {
    session_event->set_response("SUCCESS\n");
  } else {
    session_event->set_response("FAILURE\n");
  }
  return rc;
}

RC ExecuteStage::do_insert(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  SessionEvent *session_event = sql_event->session_event();
  Session *session = session_event->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  CLogManager *clog_manager = db->get_clog_manager();

  if (stmt == nullptr) {
    LOG_WARN("cannot find statement");
    return RC::GENERIC_ERROR;
  }

  InsertStmt *insert_stmt = (InsertStmt *)stmt;
  InsertOperator insert_oper(insert_stmt, trx);
  RC rc = insert_oper.open();
  
  if (rc == RC::SUCCESS) {
    if (!session->is_trx_multi_operation_mode()) {
      CLogRecord *clog_record = nullptr;
      rc = clog_manager->clog_gen_record(CLogType::REDO_MTR_COMMIT, trx->get_current_id(), clog_record);
      if (rc != RC::SUCCESS || clog_record == nullptr) {
        session_event->set_response("FAILURE\n");
        return rc;
      }

      rc = clog_manager->clog_append_record(clog_record);
      if (rc != RC::SUCCESS) {
        session_event->set_response("FAILURE\n");
        return rc;
      } 

      trx->next_current_id();
      session_event->set_response("SUCCESS\n");
    } else {
      session_event->set_response("SUCCESS\n");
    }
  } else {
    session_event->set_response("FAILURE\n");
  }
  return rc;
}

RC ExecuteStage::do_delete(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  SessionEvent *session_event = sql_event->session_event();
  Session *session = session_event->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  CLogManager *clog_manager = db->get_clog_manager();

  if (stmt == nullptr) {
    LOG_WARN("cannot find statement");
    return RC::GENERIC_ERROR;
  }

  DeleteStmt *delete_stmt = (DeleteStmt *)stmt;
  TableScanOperator scan_oper(delete_stmt->table());
  PredicateOperator pred_oper(delete_stmt->filter_stmt());
  pred_oper.add_child(&scan_oper);
  DeleteOperator delete_oper(delete_stmt, trx);
  delete_oper.add_child(&pred_oper);

  RC rc = delete_oper.open();
  if (rc != RC::SUCCESS) {
    session_event->set_response("FAILURE\n");
  } else {
    session_event->set_response("SUCCESS\n");
    if (!session->is_trx_multi_operation_mode()) {
      CLogRecord *clog_record = nullptr;
      rc = clog_manager->clog_gen_record(CLogType::REDO_MTR_COMMIT, trx->get_current_id(), clog_record);
      if (rc != RC::SUCCESS || clog_record == nullptr) {
        session_event->set_response("FAILURE\n");
        return rc;
      }

      rc = clog_manager->clog_append_record(clog_record);
      if (rc != RC::SUCCESS) {
        session_event->set_response("FAILURE\n");
        return rc;
      } 

      trx->next_current_id();
      session_event->set_response("SUCCESS\n");
    }
  }
  return rc;
}

RC ExecuteStage::do_update(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  SessionEvent *session_event = sql_event->session_event();
  Session *session = session_event->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  CLogManager *clog_manager = db->get_clog_manager();

  if (stmt == nullptr) {
    LOG_WARN("cannot find statement");
    return RC::GENERIC_ERROR;
  }
  
  RC rc = RC::SUCCESS;
  UpdateStmt *update_stmt = (UpdateStmt *)stmt;
  for (int i = 0; i < update_stmt->col_val_size(); ++i) {
    // Field field = update_stmt->field(i);
    Value value = update_stmt->value(i);
    if (value.type == SUBQUERY) {
      SelectStmt *select_stmt = (SelectStmt*)value.data;
      SQLStageEvent *sub_event = new SQLStageEvent(sql_event->session_event(), sql_event->sql());
      sub_event->set_stmt(select_stmt);
      sub_event->set_query(sql_event->query());
      std::vector<ResTuple> ret;
      rc = do_select(sub_event, ret, false);
      if (rc != RC::SUCCESS || ret.size() != 1 || ret[0].size() > 1) {
        session_event->set_response("FAILURE\n");
        return RC::GENERIC_ERROR;
      }
      update_stmt->value(i).type = ret[0].cell(0).attr_type();
      if (update_stmt->value(i).type != NULLS) {
        memcpy(update_stmt->value(i).data, ret[0].cell(0).data(), ret[0].cell(0).length());
      } else {
        update_stmt->value(i).data = nullptr;
      }
    }
  }
  UpdateOperator update_oper(update_stmt, trx);
  TableScanOperator scan_oper(update_stmt->table());
  FilterStmt *update_filter = update_stmt->filter_stmt();
  // for (FilterUnit *unit : update_filter->filter_units()) {
  //   if(unit->left()->type() == ExprType::VALUE){
  //     TupleCell cell;
  //     static_cast<ValueExpr *>(unit->left())->get_tuple_cell(cell);
  //     if (cell.attr_type() == NULLS && unit->comp() == EQUAL_TO) {
  //       unit->set_comp(IS_NULL);
  //     } else if(cell.attr_type() == NULLS && unit->comp() == NOT_EQUAL) {
  //       unit->set_comp(IS_NOT_NULL);
  //     }
  //   }
  //   if(unit->right()->type() == ExprType::VALUE){
  //     TupleCell cell;
  //     static_cast<ValueExpr *>(unit->right())->get_tuple_cell(cell);
  //     if (cell.attr_type() == NULLS && unit->comp() == EQUAL_TO) {
  //       unit->set_comp(IS_NULL);
  //     } else if(cell.attr_type() == NULLS && unit->comp() == NOT_EQUAL) {
  //       unit->set_comp(IS_NOT_NULL);
  //     }
  //   }

  // }
  if(update_filter != nullptr){
    PredicateOperator pred_oper(update_stmt->filter_stmt());
    pred_oper.add_child(&scan_oper);
    update_oper.add_child(&pred_oper);
    rc = update_oper.open();
  } else{
    update_oper.add_child(&scan_oper);
    rc = update_oper.open();
  }
  
  if (rc == RC::SUCCESS) {
    if (!session->is_trx_multi_operation_mode()) {
      CLogRecord *clog_record = nullptr;
      rc = clog_manager->clog_gen_record(CLogType::REDO_MTR_COMMIT, trx->get_current_id(), clog_record);
      if (rc != RC::SUCCESS || clog_record == nullptr) {
        session_event->set_response("FAILURE\n");
        return rc;
      }

      rc = clog_manager->clog_append_record(clog_record);
      if (rc != RC::SUCCESS) {
        session_event->set_response("FAILURE\n");
        return rc;
      } 

      trx->next_current_id();
      session_event->set_response("SUCCESS\n");
    } else {
      session_event->set_response("SUCCESS\n");
    }
  } else {
    session_event->set_response("FAILURE\n");
  }

  return RC::SUCCESS;
}

RC ExecuteStage::do_begin(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;
  SessionEvent *session_event = sql_event->session_event();
  Session *session = session_event->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  CLogManager *clog_manager = db->get_clog_manager();

  session->set_trx_multi_operation_mode(true);

  CLogRecord *clog_record = nullptr;
  rc = clog_manager->clog_gen_record(CLogType::REDO_MTR_BEGIN, trx->get_current_id(), clog_record);
  if (rc != RC::SUCCESS || clog_record == nullptr) {
    session_event->set_response("FAILURE\n");
    return rc;
  }

  rc = clog_manager->clog_append_record(clog_record);
  if (rc != RC::SUCCESS) {
    session_event->set_response("FAILURE\n");
  } else {
    session_event->set_response("SUCCESS\n");
  }

  return rc;
}

RC ExecuteStage::do_commit(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;
  SessionEvent *session_event = sql_event->session_event();
  Session *session = session_event->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  CLogManager *clog_manager = db->get_clog_manager();

  session->set_trx_multi_operation_mode(false);

  CLogRecord *clog_record = nullptr;
  rc = clog_manager->clog_gen_record(CLogType::REDO_MTR_COMMIT, trx->get_current_id(), clog_record);
  if (rc != RC::SUCCESS || clog_record == nullptr) {
    session_event->set_response("FAILURE\n");
    return rc;
  }

  rc = clog_manager->clog_append_record(clog_record);
  if (rc != RC::SUCCESS) {
    session_event->set_response("FAILURE\n");
  } else {
    session_event->set_response("SUCCESS\n");
  }

  trx->next_current_id();

  return rc;
}

RC ExecuteStage::do_clog_sync(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;
  SessionEvent *session_event = sql_event->session_event();
  Db *db = session_event->session()->get_current_db();
  CLogManager *clog_manager = db->get_clog_manager();

  rc = clog_manager->clog_sync();
  if (rc != RC::SUCCESS) {
    session_event->set_response("FAILURE\n");
  } else {
    session_event->set_response("SUCCESS\n");
  }

  return rc;
}

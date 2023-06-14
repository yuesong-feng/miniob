#pragma once
#include "execute_stage.h"
#include "common/io/io.h"
#include "common/log/log.h"
#include "common/lang/defer.h"
#include "common/seda/timer_stage.h"
#include "common/lang/string.h"
#include "session/session.h"
#include "event/storage_event.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "sql/expr/tuple.h"
#include "sql/expr/res_tuple.h"
#include "sql/operator/table_scan_operator.h"
#include "sql/operator/index_scan_operator.h"
#include "sql/operator/predicate_operator.h"
#include "sql/operator/insert_operator.h"
#include "sql/operator/delete_operator.h"
#include "sql/operator/update_operator.h"
#include "sql/operator/project_operator.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/common/table.h"
#include "storage/common/field.h"
#include "storage/index/index.h"
#include "storage/default/default_handler.h"
#include "storage/common/condition_filter.h"
#include "storage/trx/trx.h"
#include "storage/clog/clog.h"
#include "util/calculator.h"
#include "util/util.h"
#include "util/comparator.h"

void end_trx_if_need(Session *session, Trx *trx, bool all_right)
{
  if (!session->is_trx_multi_operation_mode()) {
    if (all_right) {
      trx->commit();
    } else {
      trx->rollback();
    }
  }
}

void print_tuple_header(std::ostream &os, std::vector<const TupleCellSpec *> header)
{
  for (int i = 0; i < header.size(); i++) {
    const TupleCellSpec *cell_spec = header[i];
    if (i != 0) {
      os << " | ";
    }
    if (cell_spec->alias()) {
      os << cell_spec->alias();
    }
  }
  if (header.size() > 0) {
    os << '\n';
  }
}

void print_tuple_header(std::ostream &os, const ProjectOperator &oper)
{
  const int cell_num = oper.tuple_cell_num();
  const TupleCellSpec *cell_spec = nullptr;
  for (int i = 1; i < cell_num; i++) {
    oper.tuple_cell_spec_at(i, cell_spec);
    if (i != 1) {
      os << " | ";
    }
    if (cell_spec->alias()) {
      os << cell_spec->alias();
    }
  }
  if (cell_num > 0) {
    os << '\n';
  }
}

void print_tuple_header(std::ostream &os, std::vector<std::string> &agg_headers) {
  for (int i = 0; i < agg_headers.size(); ++i) {
    std::string str = agg_headers[i];
    if (i != 0) {
      os << " | ";
    }
    os << str;
  }
  if (agg_headers.size() > 0) {
    os << '\n';
  }
}

void print_tuple_header(std::ostream &os, std::vector<Field> &query_headers) {
  for (int i = 0; i < query_headers.size(); i++) {
    const char* str = query_headers[i].field_name();
    if (i != 0) {
      os << " | ";
    }
    os << str;
  }
  if (query_headers.size() > 0) {
    os << '\n';
  }
}

void print_tuples(std::ostream &os, std::vector<std::vector<TupleCell>> &tuple_vec){
  for (std::vector<TupleCell> &tv : tuple_vec) {
    bool first_field = true;
    for(int i = 0; i < tv.size(); ++i) {
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      tv[i].to_string(os);
    }
    os << std::endl;
  }
}

void tuple_to_string(std::ostream &os, const Tuple &tuple)
{
  TupleCell cell;
  RC rc = RC::SUCCESS;
  bool first_field = true;
  for (int i = 0; i < tuple.cell_num(); i++) {
    rc = tuple.cell_at(i, cell);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to fetch field of cell. index=%d, rc=%s", i, strrc(rc));
      break;
    }

    if (!first_field) {
      os << " | ";
    } else {
      first_field = false;
    }
    cell.to_string(os);
  }
}

void print_tuples(std::ostream &os, std::vector<ResTuple> &results) {
  for (ResTuple &rt : results) {
    bool first_field = true;
    for(int i = 0; i < rt.size(); ++i) {
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      if (rt.cell(i).attr_type() == NULLS) {
        rt.cell(i).set_type(CHARS);
        rt.cell(i).set_data("NULL");
      }
      rt.cell(i).to_string(os);
    }
    os << std::endl;
  }
}

void print_agg_expression_tuples(std::ostream &os, std::vector<ResTuple> &results, std::vector<Field> &query_headers) {
  if(results.size()==0) return;
  std::vector<int> cell_index(query_headers.size(), -1);
  for(int i=0; i<query_headers.size(); ++i){
    if (query_headers[i].attr_type() == EXPRESSION) {
      cell_index[i] = -10;
      continue;
    }
    for(int j=0; j<results[0].size(); ++j){
      if(strcmp(query_headers[i].field_name(), results[0].field(j).field_name())==0){
        cell_index[i] = j;
      }
    }
  }

  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  for (ResTuple &rt : results) {
    bool first_field = true;
    for(int i=0; i<query_headers.size(); ++i){
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      if (cell_index[i] != -10) {
        if (rt.cell(cell_index[i]).attr_type() == NULLS) {
          rt.cell(cell_index[i]).set_type(CHARS);
        }
        rt.cell(cell_index[i]).to_string(os);
      } else {
        // expression
        std::string expr = query_headers[i].field_name();
        for (int j = 0; j < rt.size(); ++j) {
          std::string field_name(rt.field(j).field_name());
          int index = -1;
          while (( index = expr.find(field_name) ) != std::string::npos) {
            // expression中有当前列，替换成值
            TupleCell cell = rt.cell(j);
            std::stringstream ss;
            cell.to_string(ss);
            std::string val = "(" + ss.str() + ")";
            expr.replace(index, field_name.size(), val);
          }
        }
        bool devided_by_zero = false;
        float val = calculate(expr, devided_by_zero);
        if (devided_by_zero) {
          os << "NULL";
        }
        else {
          os << val;
        }
      }
    }
    os << std::endl;
  }
}

void print_tuples(std::ostream &os, std::vector<ResTuple> &results, std::vector<Field> &query_headers) {
  if(results.size()==0) return;
  std::vector<int> cell_index(query_headers.size(), -1);
  for(int i=0; i<query_headers.size(); ++i){
    if (query_headers[i].attr_type() == EXPRESSION) {
      cell_index[i] = -10;
      continue;
    }
    for(int j=0; j<results[0].size(); ++j){
      if(strcmp(query_headers[i].field_name(), results[0].field(j).field_name())==0){
        cell_index[i] = j;
      }
    }
  }

  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  for (ResTuple &rt : results) {
    bool first_field = true;
    for(int i=0; i<query_headers.size(); ++i){
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      if (cell_index[i] != -10) {
        if (rt.cell(cell_index[i]).attr_type() == NULLS) {
          rt.cell(cell_index[i]).set_type(CHARS);
        }
        rt.cell(cell_index[i]).to_string(os);
      } else {
        // expression
        std::string expr = query_headers[i].field_name();
        for (int j = 0; j < rt.size(); ++j) {
          std::string field_name(rt.field(j).field_name());
          int index = -1;
          while (( index = expr.find(field_name) ) != std::string::npos) {
            // expression中有当前列，替换成值
            TupleCell cell = rt.cell(j);
            std::stringstream ss;
            cell.to_string(ss);
            std::string val = "(" + ss.str() + ")";
            expr.replace(index, field_name.size(), val);
          }
        }
        bool devided_by_zero = false;
        float val = calculate(expr, devided_by_zero);
        if (devided_by_zero) {
          os << "NULL";
        }
        else {
          os << val;
        }
      }
    }
    os << std::endl;
  }
}

void print_tuples(std::ostream &os, std::vector<ResTuple> &results, std::vector<Field> &header_names, std::vector<FilterUnit*> &expression_filter_units) {
  if(results.size()==0) return;
  std::vector<int> cell_index(header_names.size(), -1);
  for(int i=0; i<header_names.size(); ++i){
    if (header_names[i].attr_type() == EXPRESSION) {
      cell_index[i] = -10;
      continue;
    }
    for(int j=0; j<results[0].size(); ++j){
      if(strcmp(header_names[i].table_name(), results[0].field(j).table_name())==0 && strcmp(header_names[i].field_name(), results[0].field(j).field_name())==0){
        cell_index[i] = j;
      }
    }
  }
  
  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  for (ResTuple &rt : results) {
    bool first_field = true;
    bool filt_by_expression = false;
    for (FilterUnit *filter_unit : expression_filter_units) {
      Expression *left = filter_unit->left();
      Expression *right = filter_unit->right();
      if (left->type() == ExprType::VALUE && right->type() == ExprType::VALUE) {
        ValueExpr *left_expr = static_cast<ValueExpr *>(filter_unit->left());
        ValueExpr *right_expr = static_cast<ValueExpr *>(filter_unit->right());
        TupleCell left_tuple, right_tuple;
        left_expr->get_tuple_cell(left_tuple);
        right_expr->get_tuple_cell(right_tuple);
        float left_val, right_val;
        // left_val
        if (left_tuple.attr_type() == EXPRESSION) {
          std::string expr = left_tuple.data();
          for (int j = 0; j < rt.size(); ++j) {
            std::string field_name = std::string(rt.field(j).table_name()) + "." + std::string(rt.field(j).field_name());
            int index = -1;
            while (( index = expr.find(field_name) ) != std::string::npos) {
              // expression中有当前列，替换成值
              TupleCell cell = rt.cell(j);
              std::stringstream ss;
              cell.to_string(ss);
              std::string val = "(" + ss.str() + ")";
              expr.replace(index, field_name.size(), val);
            }
          }
          bool devided_by_zero = false;
          left_val = calculate(expr, devided_by_zero);
          if (devided_by_zero) {
            filt_by_expression = true;
            break;
          }
        }
        else {
          switch(left_tuple.attr_type()) {
            case INTS: {
              left_val = *(int*)left_tuple.data();
              break;
            }
            case FLOATS: {
              left_val = *(float*)left_tuple.data();
              break;
            }
            case CHARS: {
              left_val = *chars2floats(left_tuple.data());
            }
          }
        }
        // right_val
        if (right_tuple.attr_type() == EXPRESSION) {
          std::string expr = right_tuple.data();
          for (int j = 0; j < rt.size(); ++j) {
            std::string field_name = std::string(rt.field(j).table_name()) + "." + std::string(rt.field(j).field_name());
            int index = -1;
            while (( index = expr.find(field_name) ) != std::string::npos) {
              // expression中有当前列，替换成值
              TupleCell cell = rt.cell(j);
              std::stringstream ss;
              cell.to_string(ss);
              std::string val = "(" + ss.str() + ")";
              expr.replace(index, field_name.size(), val);
            }
          }
          bool devided_by_zero = false;
          right_val = calculate(expr, devided_by_zero);
          if (devided_by_zero) {
            filt_by_expression = true;
            break;
          }
        }
        else {
          switch(right_tuple.attr_type()) {
            case INTS: {
              right_val = *(int*)right_tuple.data();
              break;
            }
            case FLOATS: {
              right_val = *(float*)right_tuple.data();
              break;
            }
            case CHARS: {
              right_val = *chars2floats(right_tuple.data());
            }
          }
        }
        // compare
        switch(filter_unit->comp()) {
          case EQUAL_TO: {
            if (compare_float(left_val, right_val)!=0) {
              filt_by_expression = true;
            }
            break;
          }
          case LESS_EQUAL: {
            if (compare_float(left_val, right_val)>0) {
              filt_by_expression = true;
            }
            break;
          }
          case NOT_EQUAL: {
            if (compare_float(left_val, right_val)==0) {
              filt_by_expression = true;
            }
            break;
          }
          case LESS_THAN: {
            if (compare_float(left_val, right_val)>=0) {
              filt_by_expression = true;
            }
            break;
          }
          case GREAT_EQUAL: {
            if (compare_float(left_val, right_val)<0) {
              filt_by_expression = true;
            }
            break;
          }
          case GREAT_THAN: {
            if (compare_float(left_val, right_val)<=0) {
              filt_by_expression = true;
            }
            break;
          }
        }
      }
      else if (left->type() == ExprType::FIELD && right->type() == ExprType::VALUE) {
        FieldExpr *left_field_expr = static_cast<FieldExpr *>(filter_unit->left());
        ValueExpr *right_expr = static_cast<ValueExpr *>(filter_unit->right());
        const Field &field = left_field_expr->field();
        TupleCell right_tuple;
        right_expr->get_tuple_cell(right_tuple);
        float left_val, right_val;
        // left_val
        for (int j = 0; j < rt.size(); ++j) {
          if (strcmp(field.table_name(), rt.field(j).table_name())==0 && strcmp(field.field_name(), rt.field(j).field_name())==0) {
            left_val = *(float*)rt.cell(j).data();
          }
        }
        // right_val
        if (right_tuple.attr_type() == EXPRESSION) {
          std::string expr = right_tuple.data();
          for (int j = 0; j < rt.size(); ++j) {
            std::string field_name = std::string(rt.field(j).table_name()) + "." + std::string(rt.field(j).field_name());
            int index = -1;
            while (( index = expr.find(field_name) ) != std::string::npos) {
              // expression中有当前列，替换成值
              TupleCell cell = rt.cell(j);
              std::stringstream ss;
              cell.to_string(ss);
              std::string val = "(" + ss.str() + ")";
              expr.replace(index, field_name.size(), val);
            }
          }
          bool devided_by_zero = false;
          right_val = calculate(expr, devided_by_zero);
          if (devided_by_zero) {
            filt_by_expression = true;
            break;
          }
        }
        else {
          LOG_WARN("right_tuple.attr_type should be EXPRESSION");
          switch(right_tuple.attr_type()) {
            case INTS: {
              right_val = *(int*)right_tuple.data();
              break;
            }
            case FLOATS: {
              right_val = *(float*)right_tuple.data();
              break;
            }
            case CHARS: {
              right_val = *chars2floats(right_tuple.data());
            }
          }
        }
        // compare
        switch(filter_unit->comp()) {
          case EQUAL_TO: {
            if (compare_float(left_val, right_val)!=0) {
              filt_by_expression = true;
            }
            break;
          }
          case LESS_EQUAL: {
            if (compare_float(left_val, right_val)>0) {
              filt_by_expression = true;
            }
            break;
          }
          case NOT_EQUAL: {
            if (compare_float(left_val, right_val)==0) {
              filt_by_expression = true;
            }
            break;
          }
          case LESS_THAN: {
            if (compare_float(left_val, right_val)>=0) {
              filt_by_expression = true;
            }
            break;
          }
          case GREAT_EQUAL: {
            if (compare_float(left_val, right_val)<0) {
              filt_by_expression = true;
            }
            break;
          }
          case GREAT_THAN: {
            if (compare_float(left_val, right_val)<=0) {
              filt_by_expression = true;
            }
            break;
          }
        }
      }
      if (filt_by_expression) {
        break;
      }
    }
    if (filt_by_expression) {
      continue;
    }
    for(int i=0; i<header_names.size(); ++i){
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      if (cell_index[i] != -10) {
        if (rt.cell(cell_index[i]).attr_type() == NULLS) {
          rt.cell(cell_index[i]).set_type(CHARS);
        }
        rt.cell(cell_index[i]).to_string(os);
      } else {
        // expression
        std::string expr = header_names[i].field_name();
        for (int j = 0; j < rt.size(); ++j) {
          std::string field_name = std::string(rt.field(j).table_name()) + "." + std::string(rt.field(j).field_name());
          int index = -1;
          while (( index = expr.find(field_name) ) != std::string::npos) {
            // expression中有当前列，替换成值
            TupleCell cell = rt.cell(j);
            std::stringstream ss;
            cell.to_string(ss);
            std::string val = "(" + ss.str() + ")";
            expr.replace(index, field_name.size(), val);
          }
        }
        bool devided_by_zero = false;
        float val = calculate(expr, devided_by_zero);
        if (devided_by_zero) {
          os << "NULL";
        }
        else {
          os << val;
        }
      }
    }
    os << std::endl;
  }
}

void print_groupby_tuples(std::ostream &os, std::vector<ResTuple> &results, std::vector<std::string> &header_names, std::vector<std::string> &results_headers) {
  if(results.size()==0) return;
  std::vector<int> cell_index(header_names.size(), -1);
  for(int i=0; i<header_names.size(); ++i){
    for(int j=0; j<results_headers.size(); ++j){
      if(strcmp(header_names[i].c_str(), results_headers[j].c_str())==0){
        cell_index[i] = j;
      }
    }
  }
  
  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  for (ResTuple &rt : results) {
    bool first_field = true;
    for(int i=0; i<header_names.size(); ++i){
      if (!first_field) {
        os << " | ";
      } else {
        first_field = false;
      }
      if (rt.cell(cell_index[i]).attr_type() == NULLS) {
        rt.cell(cell_index[i]).set_type(CHARS);
      }
      rt.cell(cell_index[i]).to_string(os);
    }
    os << std::endl;
  }
}

void print_fields(std::ostream &os, std::vector<std::pair<const char*, const char*>> &fields) {
  for (int i = 0; i < fields.size(); i++) {
    std::string table_name = fields[i].first;
    std::string field_name = fields[i].second;
    std::string str = table_name + "." + field_name;    
    if (i != 0) {
      os << " | ";
    }
    os << str;
  }
  if (fields.size() > 0) {
    os << '\n';
  }
}

void print_tuple_header(std::ostream &os, std::vector<ResTuple> &results) {
  if(results.size() == 0) return;
  for (int i = 0; i < results[0].fields().size(); i++) {
    std::string str;
    std::string alias = results[0].fields()[i].meta()->alias_string();
    if (alias.size() > 0 && (strcmp(alias.c_str(), "asc") != 0 && strcmp(alias.c_str(), "desc") != 0)) {
      str = std::string(results[i].fields()[i].meta()->alias());
    } else {
      str = results[0].fields()[i].field_name();
    }
    if (i != 0) {
      os << " | ";
    }
    os << str;
  }
  if (results[0].fields().size() > 0) {
    os << '\n';
  }
}

void print_tuple_header_with_table(std::ostream &os, std::vector<Field> &query_headers) {
  // for (int i = 0, j = query_headers.size() - 1; i < j; i++, j--) std::swap(query_headers[i], header_names[j]);
  for (int i = 0; i < query_headers.size(); ++i) {
    if (i != 0) {
      os << " | ";
    }
    if (query_headers[i].table_name()!=nullptr) {
      os << query_headers[i].table_name() << ".";
    }
    os << query_headers[i].field_name();
  }
  if (query_headers.size() > 0) {
    os << '\n';
  }
} 

ResTuple two_tuple_cast(ResTuple &tp1, ResTuple &tp2) {
  ResTuple ret;
  
  for (int i = 0; i < tp1.size(); ++i) {
    ret.add_cell(tp1.cell(i));
    ret.add_field(tp1.field(i));
  }
  for (int i = 0; i < tp2.size(); ++i) {
    ret.add_cell(tp2.cell(i));
    ret.add_field(tp2.field(i));
  }
  return ret;
}

void filter_by_having(std::vector<ResTuple> &result, FilterUnit *filter_unit, bool multi_table) {
  if(result.size()==0) return;
  CompOp comp = filter_unit->comp();
  Expression *left = filter_unit->left();
  Expression *right = filter_unit->right();
  FieldExpr &left_field_expr = *(FieldExpr *)left;
  ValueExpr &right_value_expr = *(ValueExpr *)right;
  const Field &left_field = left_field_expr.field();
  TupleCell cell;
  right_value_expr.get_tuple_cell(cell);

  int idx = -1;
  for(int i=0; i<result[0].size(); ++i){
    if (strcmp(result[0].field(i).field_name(), "*")==0 && strcmp(left_field.field_name(), "__trx")==0 
    && strcmp(result[0].field(i).meta()->agg_func(), "count")==0 && strcmp(left_field.meta()->agg_func(), "count")==0) {
      // ????????��?????????????multi_table?true
      idx = i;
      break;
    }
    if (multi_table && (result[0].field(i).table_name()==nullptr || strcmp(result[0].field(i).table_name(), left_field.table_name())!=0)) {
        continue;
    }
    if(strcmp(result[0].field(i).field_name(), left_field.field_name())==0 
    && strcmp(result[0].field(i).meta()->agg_func(), left_field.meta()->agg_func())==0){
      idx = i;
      break;
    }
  }
  if(idx==-1){
    LOG_WARN("field in having filter not exit in result");
    return;
  }

  std::vector<ResTuple> ret;
  for(ResTuple &t : result) {
    switch(comp){
      case EQUAL_TO: {
        if(t.cell(idx).compare(cell)==0){
          ret.push_back(t);
        }
        break;
      }
      case LESS_EQUAL: {
        if(t.cell(idx).compare(cell)<=0){
          ret.push_back(t);
        }
        break;
      }
      case NOT_EQUAL: {
        if(t.cell(idx).compare(cell)!=0){
          ret.push_back(t);
        }
        break;
      }
      case LESS_THAN: {
        if(t.cell(idx).compare(cell)<0){
          ret.push_back(t);
        }
        break;
      }
      case GREAT_EQUAL: {
        if(t.cell(idx).compare(cell)>=0){
          ret.push_back(t);
        }
        break;
      }
      case GREAT_THAN: {
        if(t.cell(idx).compare(cell)>0){
          ret.push_back(t);
        }
        break;
      }
      case LIKE: {
        if(t.cell(idx).like(cell)){
          ret.push_back(t);
        }
        break;
      }
      case NOT_LIKE: {
        if(!t.cell(idx).like(cell)){
          ret.push_back(t);
        }
        break;
      }
      case NO_OP: {
        LOG_WARN("should not happen. comp=%d", comp);
      }
    }
  }
  
  result = ret;

  return;
}

// ???????????????filter_unit???��???
void filter_by_join_field(std::vector<ResTuple> &res1, std::vector<ResTuple> &res2, FilterUnit *filter_unit) {
  if(res1.size()==0 || res2.size()==0) return;
  CompOp comp = filter_unit->comp();
  Expression *left = filter_unit->left();
  Expression *right = filter_unit->right();
  FieldExpr &left_field_expr = *(FieldExpr *)left;
  FieldExpr &right_field_expr = *(FieldExpr *)right;
  const Field &left_field = left_field_expr.field();
  const Field &right_field = right_field_expr.field();

  int left_index = -1, right_index = -1;
  for(int i=0; i<res1[0].size(); ++i){
    if(strcmp(res1[0].field(i).table_name(), left_field.table_name())==0 && strcmp(res1[0].field(i).field_name(), left_field.field_name())==0){
      left_index = i;
      break;
    }
  }
  if(left_index==-1){
    return;
  }

  for(int i=0; i<res2[0].size(); ++i){
    if(strcmp(res2[0].field(i).table_name(), right_field.table_name())==0 && strcmp(res2[0].field(i).field_name(), right_field.field_name())==0){
      right_index = i;
      break;
    }
  }

  if(right_index==-1){
    return;
  }

  std::vector<bool> remain1(res1.size(), false), remain2(res2.size(), false);
  std::vector<ResTuple> ret;
  for(int i=0; i<res1.size(); ++i) {
    ResTuple &t1 = res1[i];
    for(int j=0; j<res2.size(); ++j) {
      ResTuple &t2 = res2[j];
      TupleCell cell1 = t1.cell(left_index);
      TupleCell cell2 = t2.cell(right_index);
      if (cell1.attr_type() == NULLS || cell2.attr_type() == NULLS) {
        continue;
      }
      switch(comp){
        case EQUAL_TO: {
          if(t1.cell(left_index).compare(t2.cell(right_index))==0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case LESS_EQUAL: {
          if(t1.cell(left_index).compare(t2.cell(right_index))<=0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case NOT_EQUAL: {
          if(t1.cell(left_index).compare(t2.cell(right_index))!=0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case LESS_THAN: {
          if(t1.cell(left_index).compare(t2.cell(right_index))<0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case GREAT_EQUAL: {
          if(t1.cell(left_index).compare(t2.cell(right_index))>=0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case GREAT_THAN: {
          if(t1.cell(left_index).compare(t2.cell(right_index))>0){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case LIKE: {
          if(t1.cell(left_index).like(t2.cell(right_index))){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case NOT_LIKE: {
          if(!t1.cell(left_index).like(t2.cell(right_index))){
            remain1[i] = true, remain2[j] = true;
            ret.push_back(std::move(two_tuple_cast(t1, t2)));
          }
          break;
        }
        case NO_OP: {
          LOG_WARN("should not happen. comp=%d", comp);
        }
      }
    }
  }

  std::vector<ResTuple> tmp1, tmp2;
  for(int i=0; i<remain1.size(); ++i){
    if(remain1[i]){
      tmp1.push_back(res1[i]);
    }
  }
  res1 = tmp1;
  for(int i=0; i<remain2.size(); ++i){
    if(remain2[i]){
      tmp2.push_back(res2[i]);
    }
  }
  res2 = tmp2;

  return;
}

// ??????????filter_unit???��???
void filter_by_join_field(std::vector<ResTuple> &result, FilterUnit *filter_unit) {
  if(result.size()==0) return;
  CompOp comp = filter_unit->comp();
  Expression *left = filter_unit->left();
  Expression *right = filter_unit->right();
  FieldExpr &left_field_expr = *(FieldExpr *)left;
  FieldExpr &right_field_expr = *(FieldExpr *)right;
  const Field &left_field = left_field_expr.field();
  const Field &right_field = right_field_expr.field();


  int left_index = -1, right_index = -1;
  for(int i=0; i<result[0].size(); ++i){
    if(strcmp(result[0].field(i).table_name(), left_field.table_name())==0 && strcmp(result[0].field(i).field_name(), left_field.field_name())==0){
      left_index = i;
      break;
    }
  }
  if(left_index==-1){
    return;
  }

  for(int i=0; i<result[0].size(); ++i){
    if(strcmp(result[0].field(i).table_name(), right_field.table_name())==0 && strcmp(result[0].field(i).field_name(), right_field.field_name())==0){
      right_index = i;
      break;
    }
  }
  if(right_index==-1){
    return;
  }
  
  std::vector<ResTuple> ret;
  for(ResTuple &t : result) {
    switch(comp){
      case EQUAL_TO: {
        if(t.cell(left_index).compare(t.cell(right_index))==0){
          ret.push_back(t);
        }
        break;
      }
      case LESS_EQUAL: {
        if(t.cell(left_index).compare(t.cell(right_index))<=0){
          ret.push_back(t);
        }
        break;
      }
      case NOT_EQUAL: {
        if(t.cell(left_index).compare(t.cell(right_index))!=0){
          ret.push_back(t);
        }
        break;
      }
      case LESS_THAN: {
        if(t.cell(left_index).compare(t.cell(right_index))<0){
          ret.push_back(t);
        }
        break;
      }
      case GREAT_EQUAL: {
        if(t.cell(left_index).compare(t.cell(right_index))>=0){
          ret.push_back(t);
        }
        break;
      }
      case GREAT_THAN: {
        if(t.cell(left_index).compare(t.cell(right_index))>0){
          ret.push_back(t);
        }
        break;
      }
      case LIKE: {
        if(t.cell(left_index).like(t.cell(right_index))){
          ret.push_back(t);
        }
        break;
      }
      case NOT_LIKE: {
        if(!t.cell(left_index).like(t.cell(right_index))){
          ret.push_back(t);
        }
        break;
      }
      case NO_OP: {
        LOG_WARN("should not happen. comp=%d", comp);
      }
    }
  }
  
  result = ret;

  return;
}

std::vector<ResTuple> two_table_cast(std::vector<ResTuple> &res1, std::vector<ResTuple> &res2, FilterStmt *filter) {
  std::vector<ResTuple> ret;
  for(ResTuple &t1 : res1) {
    for(ResTuple &t2 : res2) {
      ret.push_back(std::move(two_tuple_cast(t1, t2)));
    }
  }
  for(FilterUnit *filter_unit : filter->filter_units()) {
    filter_by_join_field(ret, filter_unit);
  }
  return ret;
}

void project_groupby_tuples(std::vector<ResTuple> &results, std::vector<std::string> &header_names, std::vector<std::string> &results_headers) {
  if(results.size()==0) return;
  std::vector<int> cell_index(header_names.size(), -1);
  for(int i=0; i<header_names.size(); ++i){
    for(int j=0; j<results_headers.size(); ++j){
      if(strcmp(header_names[i].c_str(), results_headers[j].c_str())==0){
        cell_index[i] = j;
      }
    }
  }
  
  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  std::vector<ResTuple> ret;
  for (ResTuple &rt : results) {
    ResTuple resTuple;
    for (int i=0; i<cell_index.size(); ++i) {
      resTuple.add_field(rt.field(cell_index[i]));
      resTuple.add_cell(rt.cell(cell_index[i]));
    }
    ret.push_back(resTuple);
  }
  results = ret;
}

void project_tuples(std::vector<ResTuple> &results, std::vector<Field> &query_headers) {
  if(results.size()==0) return;
  std::vector<int> cell_index(query_headers.size(), -1);
  for(int i=0; i<query_headers.size(); ++i){
    for(int j=0; j<results[0].size(); ++j){
      if(strcmp(query_headers[i].field_name(), results[0].field(j).field_name())==0){
        cell_index[i] = j;
      }
    }
  }
  
  for(int i=0; i<cell_index.size(); ++i){
    if(cell_index[i] == -1){
      LOG_WARN("query_field not exits");
      return;
    }
  }

  std::vector<ResTuple> ret;
  for (ResTuple &rt : results) {
    ResTuple resTuple;
    for (int i=0; i<cell_index.size(); ++i) {
      resTuple.add_field(rt.field(cell_index[i]));
      resTuple.add_cell(rt.cell(cell_index[i]));
    }
    ret.push_back(resTuple);
  }
  results = ret;
}

bool expression_to_aggregation(SelectStmt* select_stmt){
  bool have_agg_expression = false;
  for (int i = 0; i < select_stmt->query_fields().size(); ++i) {
    Field field = select_stmt->query_fields()[i];
    if (field.attr_type() != EXPRESSION) continue;
    // 解析到agg_fields
    std::string expr = field.field_name();
    int index = -1;
    while ((index = expr.find("min(")) != std::string::npos) {
      have_agg_expression = true;
      RelAttr attr;
      attr.agg_func = strdup("min");
      int rbrace = expr.find(")", index + 4);
      std::string attr_name = expr.substr(index + 4, rbrace - index - 4);
      attr.attribute_name = strdup(attr_name.c_str());
      select_stmt->agg_fields().push_back(attr);
      expr.replace(index, 4, "min_(");
    }
    while ((index = expr.find("max(")) != std::string::npos) {
      have_agg_expression = true;
      RelAttr attr;
      attr.agg_func = strdup("max");
      int rbrace = expr.find(")", index + 4);
      std::string attr_name = expr.substr(index + 4, rbrace - index - 4);
      attr.attribute_name = strdup(attr_name.c_str());
      select_stmt->agg_fields().push_back(attr);
      expr.replace(index, 4, "max_(");
    }
    while ((index = expr.find("sum(")) != std::string::npos) {
      have_agg_expression = true;
      RelAttr attr;
      attr.agg_func = strdup("sum");
      int rbrace = expr.find(")", index + 4);
      std::string attr_name = expr.substr(index + 4, rbrace - index - 4);
      attr.attribute_name = strdup(attr_name.c_str());
      select_stmt->agg_fields().push_back(attr);
      expr.replace(index, 4, "sum_(");
    }
    while ((index = expr.find("avg(")) != std::string::npos) {
      have_agg_expression = true;
      RelAttr attr;
      attr.agg_func = strdup("avg");
      int rbrace = expr.find(")", index + 4);
      std::string attr_name = expr.substr(index + 4, rbrace - index - 4);
      attr.attribute_name = strdup(attr_name.c_str());
      select_stmt->agg_fields().push_back(attr);
      expr.replace(index, 4, "avg_(");
    }
    while ((index = expr.find("count(")) != std::string::npos) {
      have_agg_expression = true;
      RelAttr attr;
      attr.agg_func = strdup("count");
      int rbrace = expr.find(")", index + 6);
      std::string attr_name = expr.substr(index + 6, rbrace - index - 6);
      attr.attribute_name = strdup(attr_name.c_str());
      select_stmt->agg_fields().push_back(attr);
      expr.replace(index, 6, "count_(");
    }
  }
  return have_agg_expression;
}
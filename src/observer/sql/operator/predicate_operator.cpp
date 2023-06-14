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
// Created by WangYunlai on 2022/6/27.
//

#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/expr/res_tuple.h"
#include "sql/expr/tuple.h"
#include "sql/operator/predicate_operator.h"
#include "sql/parser/parse_defs.h"
#include "storage/record/record.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/common/field.h"
#include "util/calculator.h"
#include "util/util.h"
#include <sstream>
#include <vector>

RC PredicateOperator::open()
{
  if (children_.size() != 1) {
    LOG_WARN("predicate operator must has one child");
    return RC::INTERNAL;
  }

  return children_[0]->open();
}

RC PredicateOperator::next()
{
  RC rc = RC::SUCCESS;
  Operator *oper = children_[0];
  
  while (RC::SUCCESS == (rc = oper->next())) {
    Tuple *tuple = oper->current_tuple();
    if (nullptr == tuple) {
      rc = RC::INTERNAL;
      LOG_WARN("failed to get tuple from operator");
      break;
    }

    if (do_predicate(static_cast<RowTuple &>(*tuple))) {
      return rc;
    }
  }
  return rc;
}

RC PredicateOperator::close()
{
  children_[0]->close();
  return RC::SUCCESS;
}

Tuple * PredicateOperator::current_tuple()
{
  return children_[0]->current_tuple();
}

bool PredicateOperator::do_predicate(RowTuple &tuple)
{
  if (filter_stmt_ == nullptr || filter_stmt_->filter_units().empty()) {
    return true;
  }

  for (const FilterUnit *filter_unit : filter_stmt_->filter_units()) {
    Expression *left_expr = filter_unit->left();
    Expression *right_expr = filter_unit->right();
    CompOp comp = filter_unit->comp();
    TupleCell left_cell;
    TupleCell right_cell;
    left_expr->get_value(tuple, left_cell);
    right_expr->get_value(tuple, right_cell);

    if (left_cell.attr_type() == EXPRESSION) {
      std::string expr_str(left_cell.data());
      // 查找expression里有没有列，如col-2>0
      for (int i = 0; i < tuple.cell_num(); ++i) {
        const TupleCellSpec *spec = nullptr;
        tuple.cell_spec_at(i, spec);
        FieldExpr *field_expr = static_cast<FieldExpr *>(spec->expression());
        // 将expresstion中的列替换成值
        std::string field_name(field_expr->field_name());
        int index = -1;
        while ((index = expr_str.find(field_name)) != std::string::npos) {
          // expression中有当前列，替换成值
          TupleCell cell;
          tuple.cell_at(i, cell);
          std::stringstream ss;
          cell.to_string(ss);
          std::string val = "(" + ss.str() + ")";
          expr_str.replace(index, field_name.size(), val);
        }
      }
      bool devided_by_zero = false;
      double cal = calculate(expr_str, devided_by_zero);
      if (devided_by_zero == true) {
        left_cell.set_type(NULLS);
        left_cell.set_data("NULL");
      } else {
        left_cell.set_type(FLOATS);
        left_cell.set_data((char*)new float(cal));
      }
    }
    if (right_cell.attr_type() == EXPRESSION) {
      std::string expr_str(right_cell.data());
      // 查找expression里有没有列，如col-2>0
      for (int i = 0; i < tuple.cell_num(); ++i) {
        const TupleCellSpec *spec = nullptr;
        tuple.cell_spec_at(i, spec);
        FieldExpr *field_expr = static_cast<FieldExpr *>(spec->expression());
        // 将expresstion中的列替换成值
        std::string field_name(field_expr->field_name());
        int index = expr_str.find(field_name);
        if (index != std::string::npos) {
          // expression中有当前列，替换成值
          TupleCell cell;
          tuple.cell_at(i, cell);
          std::stringstream ss;
          cell.to_string(ss);
          std::string val = "(" + ss.str() + ")";
          expr_str.replace(index, field_name.size(), val);
        }
      }
      bool devided_by_zero = false;
      double cal = calculate(expr_str, devided_by_zero);
      if (devided_by_zero == true) {
        right_cell.set_type(NULLS);
        right_cell.set_data("NULL");
      } else {
        right_cell.set_type(FLOATS);
        right_cell.set_data((char*)new float(cal));
      }
    }


    if (left_expr->type() == ExprType::FIELD && left_cell.attr_type() != NULLS && right_cell.attr_type() == NULLS) {
      //Tuple * temp_tuple = project_oper.current_tuple();
      TupleCell null_cell;
      tuple.cell_at(1, null_cell);
      const char *null_data = null_cell.data();
      FieldExpr *left_field_expr = static_cast<FieldExpr *>(left_expr);
      Field left_field = left_field_expr->field();
      const FieldMeta *left_field_meta = left_field.meta();
      uint8_t index_col;
      index_col = (left_field_meta->offset())/4-2;
      if ((null_data[(index_col)/8] & (0x01 << (7-(index_col)%8))) != 0) {
        if (comp == IS_NULL) {
          return true;
        }
        else if (comp == IS_NOT_NULL) {
          return false;
        }
      } 
    }
    if (right_expr->type() == ExprType::FIELD && left_cell.attr_type() == NULLS && right_cell.attr_type() != NULLS) {
      TupleCell null_cell;
      tuple.cell_at(1, null_cell);
      const char *null_data = null_cell.data();
      FieldExpr *right_field_expr = static_cast<FieldExpr *>(right_expr);
      Field right_field = right_field_expr->field();
      const FieldMeta *right_field_meta = right_field.meta();
      uint8_t index_col;
      index_col = (right_field_meta->offset())/4-2;
      if ((null_data[(index_col)/8] & (0x01 << (7-(index_col)%8))) != 0) {
        if (comp == IS_NULL) {
          return true;
        }
        else if (comp == IS_NOT_NULL) {
          return false;
        }
      } 
    }
    const int compare = left_cell.compare(right_cell);
    bool filter_result = false;

    if ((left_cell.attr_type() == NULLS || right_cell.attr_type() == NULLS)) {
      switch (comp) {
        case IS_NULL:{
          filter_result = (compare == 0);
        } break;
        case IS_NOT_NULL:{
          filter_result = (compare != 0);
        } break;
        default: {
          LOG_WARN("invalid compare type: %d", comp);
        }break;
      }
      return filter_result;
    }

    // right_cell是一个sub-query的计算结果，std::vector<ResTuple>
    if (right_cell.attr_type() == SUBQUERY) {
      switch (comp) {
      case IN_SUBQUERY: {
        // std::vector<ResTuple> *ret = static_cast<std::vector<ResTuple> *>(right_cell.data());
      } break;
      case NOT_IN_SUBQUERY: {

      }break;
      default: {
        LOG_WARN("invalid compare type: %d", comp);
      }break;
      }
    }

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
    if (!filter_result) {
      return false;
    }
  }
  return true;
}

// int PredicateOperator::tuple_cell_num() const
// {
//   return children_[0]->tuple_cell_num();
// }
// RC PredicateOperator::tuple_cell_spec_at(int index, TupleCellSpec &spec) const
// {
//   return children_[0]->tuple_cell_spec_at(index, spec);
// }

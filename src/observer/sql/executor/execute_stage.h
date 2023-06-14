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
// Created by Longda on 2021/4/13.
//

#ifndef __OBSERVER_SQL_EXECUTE_STAGE_H__
#define __OBSERVER_SQL_EXECUTE_STAGE_H__

#include "common/seda/stage.h"
#include "sql/parser/parse.h"
#include "rc.h"
#include "sql/parser/parse_defs.h"
#include <vector>

class SQLStageEvent;
class SessionEvent;
class SelectStmt;
class TupleCell;
class TupleCellSpec;
class Field;
class Table;
class FilterStmt;
class ResTuple;

class ExecuteStage : public common::Stage {
public:
  ~ExecuteStage();
  static Stage *make_stage(const std::string &tag);

protected:
  // common function
  ExecuteStage(const char *tag);
  bool set_properties() override;

  bool initialize() override;
  void cleanup() override;
  void handle_event(common::StageEvent *event) override;
  void callback_event(common::StageEvent *event, common::CallbackContext *context) override;

  void handle_request(common::StageEvent *event);
  RC do_help(SQLStageEvent *session_event);
  RC do_create_table(SQLStageEvent *sql_event);
  RC do_create_index(SQLStageEvent *sql_event);
  RC do_show_index(SQLStageEvent *sql_event);
  RC do_show_tables(SQLStageEvent *sql_event);
  RC do_drop_table(SQLStageEvent *sql_event);
  RC do_desc_table(SQLStageEvent *sql_event);
  RC do_select(SQLStageEvent *sql_event, std::vector<ResTuple> &ret_res, bool output);
  RC do_select_tables(SQLStageEvent *sql_event);
  RC do_subset(std::vector<ResTuple> &results, const std::vector<Subset> &subsets);
  RC do_aggregation(std::vector<ResTuple> &results, const std::vector<RelAttr> &agg_fields, std::vector<std::string> &agg_headers, bool multi_table);
  RC do_group_by(std::vector<ResTuple> &results, const std::vector<RelAttr> &agg_fields, const std::vector<Field> &group_by_fields, std::vector<std::string> &headers, std::vector<std::string> &results_headers, bool multi_table, FilterStmt *having_filter);
  RC do_join(std::vector<std::vector<ResTuple>> &results, std::vector<const char*> &table_names, FilterStmt *where_filter,  std::vector<FilterStmt *> &join_filters, std::vector<ResTuple> &out_tuples);
  RC do_order_by(const std::vector<Field> &order_by_fields, std::vector<ResTuple> &results);
  RC do_insert(SQLStageEvent *sql_event);
  RC do_delete(SQLStageEvent *sql_event);
  RC do_update(SQLStageEvent *sql_event);
  RC do_begin(SQLStageEvent *sql_event);
  RC do_commit(SQLStageEvent *sql_event);
  RC do_clog_sync(SQLStageEvent *sql_event);

protected:
private:
  Stage *default_storage_stage_ = nullptr;
  Stage *mem_storage_stage_ = nullptr;
};

#endif  //__OBSERVER_SQL_EXECUTE_STAGE_H__

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

#include "rc.h"
#include "common/log/log.h"
#include "sql/parser/parse.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/update_stmt.h"

RC Stmt::create_stmt(Db *db,  Query &query, Stmt *&stmt)
{
  stmt = nullptr;

  switch (query.flag) {
  case SCF_INSERT: {
      return InsertStmt::create(db, query.sstr.insertion, stmt);
    }
    break;
  case SCF_DELETE: {
      return DeleteStmt::create(db, query.sstr.deletion, stmt);   
    }
  case SCF_SELECT: {
    for (int i = 0; i < query.sstr.selection.condition_num; ++i) {
      if (query.sstr.selection.conditions[i].right_is_attr == 0 && query.sstr.selection.conditions[i].right_value.type == SUBQUERY) {
        std::string sql((char*)query.sstr.selection.conditions[i].right_value.data);
        sql = sql.substr(1, sql.size() - 2);
        sql += ";";
        Query *new_query = query_create();
        RC rc = parse(sql.c_str(), new_query);
        if (rc != RC::SUCCESS) {
          LOG_WARN("parse sub-query failed");
          return rc;
        }
        Stmt *new_stmt = nullptr;
        rc = Stmt::create_stmt(db, *new_query, new_stmt);
        if (rc != RC::SUCCESS) {
          LOG_WARN("create sub-query select stmt failed");
          return rc;
        }
        query.sstr.selection.conditions[i].right_value.data = new_stmt;
      } 
      if (query.sstr.selection.conditions[i].left_is_attr == 0 && query.sstr.selection.conditions[i].left_value.type == SUBQUERY) {
        std::string sql((char*)query.sstr.selection.conditions[i].left_value.data);
        sql = sql.substr(1, sql.size() - 2);
        sql += ";";
        Query *new_query = query_create();
        RC rc = parse(sql.c_str(), new_query);
        if (rc != RC::SUCCESS) {
          LOG_WARN("parse sub-query failed");
          return rc;
        }
        Stmt *new_stmt = nullptr;
        rc = Stmt::create_stmt(db, *new_query, new_stmt);
        if (rc != RC::SUCCESS) {
          LOG_WARN("create sub-query select stmt failed");
          return rc;
        }
        query.sstr.selection.conditions[i].left_value.data = new_stmt;
      } 
      // if (query.sstr.selection.conditions[i].left_is_attr == 0 && query.sstr.selection.conditions[i].right_is_attr == 0
      //     && query.sstr.selection.conditions[i].left_value.type == SUBQUERY && query.sstr.selection.conditions[i].right_value.type == SUBQUERY) {
            
      //     }
    }
    return SelectStmt::create(db, query.sstr.selection, stmt);
  }
  case SCF_UPDATE: {
    // int subindex = 0;
    for (int i = 0; i < query.sstr.update.value_num; ++i) {
      if (query.sstr.update.values[i].type == SUBQUERY) {
        std::string sql((char*)query.sstr.update.values[i].data);
        sql = sql.substr(1, sql.size() - 2);
        sql += ";";
        Query *new_query = query_create();
        RC rc = parse(sql.c_str(), new_query);
        if (rc != RC::SUCCESS) {
          LOG_WARN("parse sub-query failed");
          return rc;
        }
        Stmt *new_stmt = nullptr;
        rc = SelectStmt::create(db, new_query->sstr.selection, new_stmt);
        if (rc != RC::SUCCESS) {
          LOG_WARN("cannot create select stmt");
          return rc;
        }
        query.sstr.update.values[i].data = new_stmt;
      }
    }
    return UpdateStmt::create(db, query.sstr.update, stmt);
  }
  default: {
      LOG_WARN("unknown query command");
    }
    break;
  }
  return RC::UNIMPLENMENT;
}



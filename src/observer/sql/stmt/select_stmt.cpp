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
// Created by Wangyunlai on 2022/6/6.
//

#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/common/db.h"
#include "storage/common/table.h"
#include <cstddef>
#include <cstring>

SelectStmt::~SelectStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

static void wildcard_fields(Table *table, std::vector<Field> &field_metas)
{
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    field_metas.push_back(Field(table, table_meta.field(i)));
  }
}

RC SelectStmt::create(Db *db, Selects &select_sql, Stmt *&stmt)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  // collect tables in `from` statement
  std::vector<Table *> tables;
  std::unordered_map<std::string, Table *> table_map;
  for (int i = select_sql.relation_num-1; i >= 0 ; --i) {
    const char *table_name = select_sql.relations[i];
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    tables.push_back(table);
    table_map.insert(std::pair<std::string, Table*>(table_name, table));
  }

  // collect tables in `join on` statement
  for (int i = 0; i < select_sql.join_num; i++) {
    const char *table_name = select_sql.joins[i].table_name;
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    tables.push_back(table);
    table_map.insert(std::pair<std::string, Table*>(table_name, table));
  }

  // group by statement 
  std::vector<Field> group_by_fields;
  for (int i = 0; i < select_sql.group_by_num; ++i) {
    const RelAttr &relation_attr = select_sql.group_by[i];
    if (common::is_blank(relation_attr.relation_name)) {
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }

      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name);
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      group_by_fields.push_back(Field(table, field_meta));
    } else {
      const char *table_name = relation_attr.relation_name;
      const char *field_name = relation_attr.attribute_name;
      auto iter = table_map.find(table_name);
      if (iter == table_map.end()) {
        LOG_WARN("no such table in from list: %s", table_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      Table *table = iter->second;
      const FieldMeta *field_meta = table->table_meta().field(field_name);
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      group_by_fields.push_back(Field(table, field_meta));
    }
  }
  
  // aggregation
  bool is_aggregation = false;
  for (int i = select_sql.attr_num-1; i >= 0; --i) {
    const RelAttr &relation_attr = select_sql.attributes[i];
    if(relation_attr.agg_func != nullptr && strcmp(relation_attr.agg_func, "expression") != 0) {
      is_aggregation = true;
    }
  }

  // collect query fields in `select` statement
  std::vector<Field> query_fields;
  std::vector<RelAttr> agg_fields;
  for (int i = select_sql.attr_num - 1; i >= 0; i--) {
    const RelAttr &relation_attr = select_sql.attributes[i];
    if (relation_attr.agg_func != nullptr && strcmp(relation_attr.agg_func, "expression") == 0) {
      FieldMeta *field_meta = new FieldMeta();
      RC rc = field_meta->init(relation_attr.attribute_name, EXPRESSION, -1, -1, true, true);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to init expression meta");
        return rc;
      }
      query_fields.push_back({nullptr, field_meta});
      continue;
    }
    if (relation_attr.agg_func == nullptr && is_aggregation && group_by_fields.size() == 0) {
      LOG_WARN("aggregation and single field mixed but without group by!");
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }

    if (common::is_blank(relation_attr.relation_name) && 0 == strcmp(relation_attr.attribute_name, "*")) {
      if (relation_attr.agg_func == nullptr) {
        for (Table *table : tables) {
          wildcard_fields(table, query_fields);
        }
      } else if(strcmp(relation_attr.agg_func, "count") == 0) {
        // TODO: count(*)
        // query_fields.push_back({table, field_meta});
        Table *table = db->find_table(select_sql.relations[0]);
        const FieldMeta *field_meta = table->table_meta().field(0);
        query_fields.push_back({table, field_meta});
      } else {
        LOG_WARN("aggregation besides count dose not support *");
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
    } else if (!common::is_blank(relation_attr.relation_name)) { // TODO
      const char *table_name = relation_attr.relation_name;
      const char *field_name = relation_attr.attribute_name;

      if (0 == strcmp(table_name, "*")) {
        if (0 != strcmp(field_name, "*")) {
          LOG_WARN("invalid field name while table is *. attr=%s", field_name);
          return RC::SCHEMA_FIELD_MISSING;
        }
        for (Table *table : tables) {
          wildcard_fields(table, query_fields);
        }
      } else {
        auto iter = table_map.find(table_name);
        if (iter == table_map.end()) {
          LOG_WARN("no such table in from list: %s", table_name);
          return RC::SCHEMA_FIELD_MISSING;
        }

        Table *table = iter->second;
        if (0 == strcmp(field_name, "*")) {
          wildcard_fields(table, query_fields);
        } else {
          const FieldMeta *field_meta = table->table_meta().field(field_name);
          if (nullptr == field_meta) {
            LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
            return RC::SCHEMA_FIELD_MISSING;
          }
          if (relation_attr.alias != nullptr) {
            field_meta->set_alias(relation_attr.alias);
          }
        query_fields.push_back(Field(table, field_meta));
        }
      }
    } else {
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }

      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name);
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      if (relation_attr.alias != nullptr) {
        field_meta->set_alias(relation_attr.alias);
      }
      query_fields.push_back(Field(table, field_meta));
    }
    if (is_aggregation) { 
      agg_fields.push_back(relation_attr);
    }
  }

  LOG_INFO("got %d tables in from stmt and %d fields in query stmt", tables.size(), query_fields.size());

  Table *default_table = nullptr;
  if (tables.size() == 1) {
    default_table = tables[0];
  }

  // create filter statement in `where` statement
  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(db, default_table, &table_map,
           select_sql.conditions, select_sql.condition_num, filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }

  // subquery
  std::vector<Subquery> subquery;
  for (int i = 0; i < select_sql.condition_num; ++i) {
    // id in (select ...)
    if (select_sql.conditions[i].left_is_attr == 1 && select_sql.conditions[i].right_is_attr == 0 && select_sql.conditions[i].right_value.type == SUBQUERY) {
      RelAttr attr = select_sql.conditions[i].left_attr;
      Value val = select_sql.conditions[i].right_value;
      Subquery sq;
      if (attr.relation_name != nullptr)
        sq.table_name = strdup(attr.relation_name);
      sq.attribute_name = strdup(attr.attribute_name);
      sq.comp = select_sql.conditions[i].comp;
      sq.subquery = (SelectStmt *)val.data;
      subquery.push_back(std::move(sq));
    } else if(select_sql.conditions[i].right_is_attr == 1 && select_sql.conditions[i].left_is_attr == 0 && select_sql.conditions[i].left_value.type == SUBQUERY) {
      // (select ...) > id
      switch (select_sql.conditions[i].comp) {
        case LESS_EQUAL:  { select_sql.conditions[i].comp = GREAT_THAN; }  break;
        case LESS_THAN:   { select_sql.conditions[i].comp = GREAT_EQUAL; } break;
        case GREAT_EQUAL: { select_sql.conditions[i].comp = LESS_THAN; }   break;
        case GREAT_THAN:  { select_sql.conditions[i].comp = LESS_EQUAL; }  break;
      }
      RelAttr attr = select_sql.conditions[i].right_attr;
      Value val = select_sql.conditions[i].left_value;
      Subquery sq;
      if (attr.relation_name != nullptr)
        sq.table_name = strdup(attr.relation_name);
      sq.attribute_name = strdup(attr.attribute_name);
      sq.comp = select_sql.conditions[i].comp;
      sq.subquery = (SelectStmt *)val.data;
      subquery.push_back(std::move(sq));
    }
  }

  // group by having
  for (int i = 0; i < select_sql.having_num; ++i) {
    if (select_sql.havings[i].left_is_attr == 1 && strcmp(select_sql.havings[i].left_attr.attribute_name, "*") == 0) {
      select_sql.havings[i].left_attr.attribute_name = strdup("__trx");
    }
    if (select_sql.havings[i].right_is_attr == 1 && strcmp(select_sql.havings[i].right_attr.attribute_name, "*") == 0) {
      select_sql.havings[i].right_attr.attribute_name = strdup("__trx");
    }
  }
  FilterStmt *having_filter = nullptr;
  rc = FilterStmt::create(db, default_table, &table_map,
           select_sql.havings, select_sql.having_num, having_filter);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }
  if (having_filter != nullptr) {
    for (int i = 0; i < having_filter->filter_units().size(); ++i) {
      FilterUnit *unit = having_filter->filter_units()[i];
      if (unit->left()->type() == ExprType::FIELD) {
        FieldExpr *left = static_cast<FieldExpr *>(unit->left());
        left->field().meta()->set_agg_func(select_sql.havings[i].left_attr.agg_func);
      }
      if (unit->right()->type() == ExprType::FIELD) {
        FieldExpr *right = static_cast<FieldExpr *>(unit->right());
        right->field().meta()->set_agg_func(select_sql.havings[i].left_attr.agg_func);
      }
    }
  }

  // create filter statements inner join
  std::vector<FilterStmt *> join_filters;
  for (int i = 0; i < select_sql.join_num; i++) {
    FilterStmt *filter = nullptr;
    RC rc = FilterStmt::create(db, default_table, &table_map,
              select_sql.joins[i].conditions, select_sql.joins[i].condition_num, filter);
    if (rc != RC::SUCCESS) {
      LOG_WARN("cannot construct filter stmt of inner join");
      return rc;
    }
    join_filters.push_back(filter);
  }

  // order by statement
  std::vector<Field> order_by_fields;
  for (int i = 0; i < select_sql.order_by_num; i++) {
    const RelAttr &relation_attr = select_sql.order_by[i];
    if (common::is_blank(relation_attr.relation_name)) {
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }

      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name);
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      field_meta->set_alias(relation_attr.alias);
      order_by_fields.push_back(Field(table, field_meta));
    } else {
      const char *table_name = relation_attr.relation_name;
      const char *field_name = relation_attr.attribute_name;
      auto iter = table_map.find(table_name);
      if (iter == table_map.end()) {
        LOG_WARN("no such table in from list: %s", table_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      Table *table = iter->second;
      const FieldMeta *field_meta = table->table_meta().field(field_name);
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      field_meta->set_alias(relation_attr.alias);
      order_by_fields.push_back(Field(table, field_meta));
    }
  }

  // sub-query
  std::vector<Subset> subsets;
  for (int i = 0; i < select_sql.subset_num; ++i) {
    subsets.push_back(select_sql.subsets[i]);
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->tables_.swap(tables);
  select_stmt->query_fields_.swap(query_fields);
  select_stmt->order_by_fields_.swap(order_by_fields);
  select_stmt->agg_fields_.swap(agg_fields);
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->join_filters_.swap(join_filters);
  select_stmt->group_by_fields_.swap(group_by_fields);
  select_stmt->having_filter_ = having_filter;
  select_stmt->subsets_.swap(subsets);
  select_stmt->subquery_.swap(subquery);
  stmt = select_stmt;
  return RC::SUCCESS;
}

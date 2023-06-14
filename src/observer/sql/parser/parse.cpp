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
// Created by Meiyi 
//

#include <cstring>
#include <mutex>
// #include <sys/_types/_size_t.h>
#include "sql/parser/parse.h"
#include "rc.h"
#include "common/log/log.h"
#include "sql/parser/parse_defs.h"

RC parse(char *st, Query *sqln);

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
void relation_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name, const char*alias, const char *agg_func)
{
  if (relation_name != nullptr) {
    relation_attr->relation_name = strdup(relation_name);
  } else {
    relation_attr->relation_name = nullptr;
  }
  relation_attr->attribute_name = strdup(attribute_name);
  if (alias != nullptr) {
    relation_attr->alias = strdup(alias);
  } else {
    relation_attr->alias = nullptr;
  }
  if (agg_func != nullptr) {
    relation_attr->agg_func = strdup(agg_func);
  } else{
    relation_attr->agg_func = nullptr;
  }
}

void relation_attr_destroy(RelAttr *relation_attr)
{
  free(relation_attr->relation_name);
  free(relation_attr->attribute_name);
  free(relation_attr->alias);
  free(relation_attr->agg_func);
  relation_attr->relation_name = nullptr;
  relation_attr->attribute_name = nullptr;
  relation_attr->alias = nullptr;
  relation_attr->agg_func = nullptr;
}

void value_init_integer(Value *value, int v)
{
  value->type = INTS;
  value->data = malloc(sizeof(v));
  memcpy(value->data, &v, sizeof(v));
}
void value_init_float(Value *value, float v)
{
  value->type = FLOATS;
  value->data = malloc(sizeof(v));
  memcpy(value->data, &v, sizeof(v));
}
void value_init_string(Value *value, const char *v)
{
  value->type = CHARS;
  value->data = strdup(v);
}
bool check_date(int y, int m, int d)
{
    static int mon[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y%400==0 || (y%100 && y%4==0));
    return y > 0
        && (m > 0)&&(m <= 12)
        && (d > 0)&&(d <= ((m==2 && leap)?1:0) + mon[m]);
}
int value_init_date(Value* value, const char* v) {
    value->type = DATES;
    int y,m,d;
    sscanf(v, "%d-%d-%d", &y, &m, &d);//not check return value eq 3, lex guarantee
    bool b = check_date(y,m,d);
    if(!b) return -1;
    int dv = y*10000+m*100+d;
    value->data = malloc(sizeof(dv));//TODO:check malloc failure
    memcpy(value->data, &dv, sizeof(dv));
    return 0;
}
void value_init_null(Value* value) {
  value->type = NULLS;
  value->data = nullptr;
}
void value_init_expression(Value *value, const char *v) {
  value->type = EXPRESSION;
  value->data = strdup(v);
}
void value_destroy(Value *value)
{
  if(value->type != NULLS && value->type != SUBQUERY) {
    free(value->data);
    value->data = nullptr;
  }
  value->type = UNDEFINED;
}

void condition_init(Condition *condition, CompOp comp, int left_is_attr, RelAttr *left_attr, Value *left_value,
    int right_is_attr, RelAttr *right_attr, Value *right_value)
{
  condition->comp = comp;
  condition->left_is_attr = left_is_attr;
  if (left_is_attr) {
    condition->left_attr = *left_attr;
  } else {
    condition->left_value = *left_value;
  }

  condition->right_is_attr = right_is_attr;
  if (right_is_attr) {
    condition->right_attr = *right_attr;
  } else {
    condition->right_value = *right_value;
  }
}
void condition_destroy(Condition *condition)
{
  if (condition->left_is_attr) {
    relation_attr_destroy(&condition->left_attr);
  } else {
    value_destroy(&condition->left_value);
  }
  if (condition->right_is_attr) {
    relation_attr_destroy(&condition->right_attr);
  } else {
    value_destroy(&condition->right_value);
  }
}

void attr_info_init(AttrInfo *attr_info, const char *name, AttrType type, size_t length, int nullable)
{
  attr_info->name = strdup(name);
  attr_info->type = type;
  attr_info->length = length;
  attr_info->nullable = nullable;
}
void attr_info_destroy(AttrInfo *attr_info)
{
  free(attr_info->name);
  attr_info->name = nullptr;
}

void join_init(Join *join, const char *table_name, Condition conditions[], size_t condition_num) 
{
  for(int i = 0; i < condition_num; i++) {
    join->conditions[i] = conditions[i];
  }
  join->condition_num = condition_num;
  join->table_name = strdup(table_name);
}

void join_destroy(Join *join) 
{
  for(int i = 0; i < join->condition_num; i++) {
    condition_destroy(&(join->conditions[i]));
  }
  free(join->table_name);
  join->table_name = nullptr;
}

void selects_init(Selects *selects, ...);
void selects_append_attribute(Selects *selects, RelAttr *rel_attr)
{
  selects->attributes[selects->attr_num++] = *rel_attr;
}
void selects_append_relation(Selects *selects, const char *relation_name)
{
  selects->relations[selects->relation_num++] = strdup(relation_name);
}
void selects_append_attributes(Selects *selects, RelAttr rel_attr[], size_t attr_num) {
  for (size_t i = 0; i < attr_num; ++i) {
    selects->attributes[i] = rel_attr[i];
  }
  selects->attr_num = attr_num;
}
void selects_append_condition(Selects *selects, Condition *condition) {
  selects->conditions[selects->condition_num++] = *condition;
}
void selects_append_subquery(Selects *selects, Condition *condition) {
  selects->conditions[selects->condition_num++] = *condition;
}

void selects_append_conditions(Selects *selects, Condition conditions[], size_t condition_num)
{
  assert(condition_num <= sizeof(selects->conditions) / sizeof(selects->conditions[0]));
  for (size_t i = 0; i < condition_num; i++) {
    selects->conditions[i] = conditions[i];
  }
  selects->condition_num = condition_num;
}
void selects_append_havings(Selects *selects, Condition havings[], size_t having_num)
{
  assert(having_num <= sizeof(selects->havings) / sizeof(selects->havings[0]));
  for (size_t i = 0; i < having_num; i++) {
    selects->havings[i] = havings[i];
  }
  selects->having_num = having_num;
}

void selects_append_joins(Selects *selects, Join joins[], size_t join_num)
{
  assert(join_num < sizeof(selects->joins) / sizeof(selects->joins[0]));
  for (size_t i = 0; i < join_num; i++) {
    selects->joins[i] = joins[i];
  }
  selects->join_num = join_num;
}

void selects_append_order_by(Selects *selects, RelAttr *order_by){
  selects->order_by[selects->order_by_num++] = *order_by;
}
void selects_append_group_by(Selects *selects, RelAttr *group_by){
  selects->group_by[selects->group_by_num++] = *group_by;
}
void selects_append_subset(Selects *selects, const char* attribute_name, const char*comp, Value values[], size_t value_num) {
  int i = selects->subset_num;
  selects->subsets[i].table_name = NULL;
  selects->subsets[i].attribute_name = strdup(attribute_name);
  selects->subsets[i].comp = IN_SUBQUERY;
  if (strcmp(comp, "not_in") == 0) {
    selects->subsets[i].comp = NOT_IN_SUBQUERY;
  }

  for (size_t j = 0; j < value_num; ++j) {
    selects->subsets[i].values[j] = values[j];
  }
  selects->subsets[i].value_num = value_num;
  selects->subset_num++;
}

void subquery_split(const char* two_subquery, char* str1, char* str2, CompOp *comp){
  int count = 0;
  int first_end = -1;
  for(int i = 0; i < strlen(two_subquery); ++i) {
    if(two_subquery[i] == '(') count++;
    else if(two_subquery[i] == ')') count--;
    if (count == 0) {
      first_end = i + 1;
      break;
    }
  }
  str1 = new char[first_end + 1];
  memset(str1, 0, first_end + 1);
  memcpy(str1, two_subquery, first_end);
  char *comp_str = new char[4];
  memset(comp, '\0', 4);
  int comp_index = 0;
  int second_start = -1;
  for (int i = first_end; i < strlen(two_subquery); ++i) {
    if (two_subquery[i] == ' ') continue;
    if(two_subquery[i] == '(') {
      second_start = i;
      break;
    }
    comp_str[comp_index++] = two_subquery[i];
  }
  str2 = new char[strlen(two_subquery) - second_start + 1];
  memset(str2, 0, strlen(two_subquery) - second_start + 1);
  memcpy(str2, two_subquery + second_start, strlen(two_subquery) - second_start);
  
  if(strncmp(comp_str, "==", 2) == 0) {
    *comp = EQUAL_TO;
  } else if(strncmp(comp_str, "<=", 2) == 0) {
    *comp = LESS_EQUAL;
  } else if(strncmp(comp_str, "<>", 2) == 0) {
    *comp = NOT_EQUAL;
  } else if(strncmp(comp_str, "<", 2) == 0) {
    *comp = LESS_THAN;
  } else if(strncmp(comp_str, ">=", 2) == 0) {
    *comp = GREAT_EQUAL;
  } else if(strncmp(comp_str, ">", 2) == 0) {
    *comp = GREAT_THAN;
  }
}
char* subquery_split_str1(const char* two_subquery, char* str1, char* str2, CompOp *comp){
  int count = 0;
  int first_end = -1;
  for(int i = 0; i < strlen(two_subquery); ++i) {
    if(two_subquery[i] == '(') count++;
    else if(two_subquery[i] == ')') count--;
    if (count == 0) {
      first_end = i + 1;
      break;
    }
  }
  str1 = new char[first_end + 1];
  memset(str1, 0, first_end + 1);
  memcpy(str1, two_subquery, first_end);
  char *comp_str = new char[4];
  memset(comp, '\0', 4);
  int comp_index = 0;
  int second_start = -1;
  for (int i = first_end; i < strlen(two_subquery); ++i) {
    if (two_subquery[i] == ' ') continue;
    if(two_subquery[i] == '(') {
      second_start = i;
      break;
    }
    comp_str[comp_index++] = two_subquery[i];
  }
  str2 = new char[strlen(two_subquery) - second_start + 1];
  memset(str2, 0, strlen(two_subquery) - second_start + 1);
  memcpy(str2, two_subquery + second_start, strlen(two_subquery) - second_start);
  
  if(strncmp(comp_str, "==", 2) == 0) {
    *comp = EQUAL_TO;
  } else if(strncmp(comp_str, "<=", 2) == 0) {
    *comp = LESS_EQUAL;
  } else if(strncmp(comp_str, "<>", 2) == 0) {
    *comp = NOT_EQUAL;
  } else if(strncmp(comp_str, "<", 2) == 0) {
    *comp = LESS_THAN;
  } else if(strncmp(comp_str, ">=", 2) == 0) {
    *comp = GREAT_EQUAL;
  } else if(strncmp(comp_str, ">", 2) == 0) {
    *comp = GREAT_THAN;
  }
  return str1;
}
char* subquery_split_str2(const char* two_subquery, char* str1, char* str2, CompOp *comp){
  int count = 0;
  int first_end = -1;
  for(int i = 0; i < strlen(two_subquery); ++i) {
    if(two_subquery[i] == '(') count++;
    else if(two_subquery[i] == ')') count--;
    if (count == 0) {
      first_end = i + 1;
      break;
    }
  }
  str1 = new char[first_end + 1];
  memset(str1, 0, first_end + 1);
  memcpy(str1, two_subquery, first_end);
  char *comp_str = new char[4];
  memset(comp, '\0', 4);
  int comp_index = 0;
  int second_start = -1;
  for (int i = first_end; i < strlen(two_subquery); ++i) {
    if (two_subquery[i] == ' ') continue;
    if(two_subquery[i] == '(') {
      second_start = i;
      break;
    }
    comp_str[comp_index++] = two_subquery[i];
  }
  str2 = new char[strlen(two_subquery) - second_start + 1];
  memset(str2, 0, strlen(two_subquery) - second_start + 1);
  memcpy(str2, two_subquery + second_start, strlen(two_subquery) - second_start);
  
  if(strncmp(comp_str, "==", 2) == 0) {
    *comp = EQUAL_TO;
  } else if(strncmp(comp_str, "<=", 2) == 0) {
    *comp = LESS_EQUAL;
  } else if(strncmp(comp_str, "<>", 2) == 0) {
    *comp = NOT_EQUAL;
  } else if(strncmp(comp_str, "<", 2) == 0) {
    *comp = LESS_THAN;
  } else if(strncmp(comp_str, ">=", 2) == 0) {
    *comp = GREAT_EQUAL;
  } else if(strncmp(comp_str, ">", 2) == 0) {
    *comp = GREAT_THAN;
  }
  return str2;
}

void selects_destroy(Selects *selects)
{
  for (size_t i = 0; i < selects->attr_num; i++) {
    relation_attr_destroy(&selects->attributes[i]);
  }
  selects->attr_num = 0;

  for (size_t i = 0; i < selects->relation_num; i++) {
    free(selects->relations[i]);
    selects->relations[i] = NULL;
  }
  selects->relation_num = 0;

  for (size_t i = 0; i < selects->condition_num; i++) {
    condition_destroy(&selects->conditions[i]);
  }
  selects->condition_num = 0;

  for (size_t i = 0; i < selects->having_num; ++i) {
    condition_destroy(&selects->havings[i]);
  }
  selects->having_num = 0;

  for (size_t i = 0; i < selects->order_by_num; i++) {
    relation_attr_destroy(&selects->order_by[i]);
  }
  selects->order_by_num = 0;

  for (size_t i = 0; i < selects->join_num; i++) {
    join_destroy(&selects->joins[i]);
  }
  selects->join_num = 0;

  for (size_t i = 0; i < selects->group_by_num; i++) {
    relation_attr_destroy(&selects->group_by[i]);
  }
  selects->group_by_num = 0;

  // join group
}

void inserts_init(Inserts *inserts, const char *relation_name, Value values[], size_t value_num)
{
  assert(value_num <= sizeof(inserts->values) / sizeof(inserts->values[0]));

  inserts->relation_name = strdup(relation_name);
  for (size_t i = 0; i < value_num; i++) {
    inserts->values[i] = values[i];
  }
  inserts->value_num = value_num;
}
void inserts_destroy(Inserts *inserts)
{
  free(inserts->relation_name);
  inserts->relation_name = nullptr;

  for (size_t i = 0; i < inserts->value_num; i++) {
    value_destroy(&inserts->values[i]);
  }
  inserts->value_num = 0;
}

void deletes_init_relation(Deletes *deletes, const char *relation_name)
{
  deletes->relation_name = strdup(relation_name);
}

void deletes_set_conditions(Deletes *deletes, Condition conditions[], size_t condition_num)
{
  assert(condition_num <= sizeof(deletes->conditions) / sizeof(deletes->conditions[0]));
  for (size_t i = 0; i < condition_num; i++) {
    deletes->conditions[i] = conditions[i];
  }
  deletes->condition_num = condition_num;
}
void deletes_destroy(Deletes *deletes)
{
  for (size_t i = 0; i < deletes->condition_num; i++) {
    condition_destroy(&deletes->conditions[i]);
  }
  deletes->condition_num = 0;
  free(deletes->relation_name);
  deletes->relation_name = nullptr;
}

void updates_init(Updates *updates, const char *relation_name, Condition conditions[], size_t condition_num)
{
  updates->relation_name = strdup(relation_name);
  // updates->attribute_name = strdup(attribute_name);
  // updates->value = *value;
  // assert(condition_num <= sizeof(updates->conditions) / sizeof(updates->conditions[0]));
  for (size_t i = 0; i < condition_num; i++) {
    updates->conditions[i] = conditions[i];
  }
  updates->condition_num = condition_num;
}


void update_append_value(Updates *updates, const char* attribute_name, Value *value){
  updates->attributes[updates->attribute_count++] = strdup(attribute_name);
  updates->values[updates->value_num++] = *value;
}
void update_append_subquery(Updates *updates, const char* attribute_name, Selects *selects) {
  updates->attributes[updates->attribute_count++] = strdup(attribute_name);
  Value *value = new Value();
  value->type = SUBQUERY;
  value->data = selects;
  updates->values[updates->value_num++] = *value;
}

void updates_destroy(Updates *updates)
{
  for (int i = 0; i < updates->attribute_count; ++i) {
    free(updates->attributes[i]);
    updates->attributes[i] = nullptr;
  }
  updates->attribute_count = 0;

  free(updates->relation_name);
  updates->relation_name = nullptr;
  for (int i = 0; i < updates->value_num; ++i) {
    value_destroy(&updates->values[i]);
  }
  updates->value_num = 0;

  for (size_t i = 0; i < updates->condition_num; i++) {
    condition_destroy(&updates->conditions[i]);
  }
  updates->condition_num = 0;
}

void create_table_append_attribute(CreateTable *create_table, AttrInfo *attr_info)
{
  create_table->attributes[create_table->attribute_count++] = *attr_info;
}

void create_table_init_name(CreateTable *create_table, const char *relation_name)
{
  create_table->relation_name = strdup(relation_name);
}

void create_table_destroy(CreateTable *create_table)
{
  for (size_t i = 0; i < create_table->attribute_count; i++) {
    attr_info_destroy(&create_table->attributes[i]);
  }
  create_table->attribute_count = 0;
  free(create_table->relation_name);
  create_table->relation_name = nullptr;
}

void drop_table_init(DropTable *drop_table, const char *relation_name)
{
  drop_table->relation_name = strdup(relation_name);
}

void drop_table_destroy(DropTable *drop_table)
{
  free(drop_table->relation_name);
  drop_table->relation_name = nullptr;
}

void create_index_init(
    CreateIndex *create_index, const char *index_name, const char *relation_name)
{
  create_index->index_name = strdup(index_name);
  create_index->relation_name = strdup(relation_name);
  // create_index->attribute_name = strdup(attr_name);
  // create_index->is_unique = is_unique;
}
void create_index_unique(CreateIndex *create_index, int is_unique) {
  create_index->is_unique = is_unique;
}
void create_index_append_attribute(CreateIndex *create_index, const char *attribute)
{
  create_index->attributes[create_index->attr_num++] = strdup(attribute);
}
void create_index_destroy(CreateIndex *create_index)
{
  free(create_index->index_name);
  free(create_index->relation_name);
  // free(create_index->attribute_name);
  for (size_t i = 0; i < create_index->attr_num; ++i) {
    free(create_index->attributes[i]);
    create_index->attributes[i] = nullptr;
  }
  create_index->index_name = nullptr;
  create_index->relation_name = nullptr;
  create_index->attr_num = 0;
  // create_index->attribute_name = nullptr;
}

void drop_index_init(DropIndex *drop_index, const char *index_name)
{
  drop_index->index_name = strdup(index_name);
}

void drop_index_destroy(DropIndex *drop_index)
{
  free((char *)drop_index->index_name);
  drop_index->index_name = nullptr;
}

void show_index_init(ShowIndex *show_index, const char *relation_name){
  show_index->relation_name = strdup(relation_name);
}
void show_index_destroy(ShowIndex *show_index){
  free((char *)show_index->relation_name);
  show_index->relation_name = nullptr;
}

void desc_table_init(DescTable *desc_table, const char *relation_name)
{
  desc_table->relation_name = strdup(relation_name);
}

void desc_table_destroy(DescTable *desc_table)
{
  free((char *)desc_table->relation_name);
  desc_table->relation_name = nullptr;
}

void load_data_init(LoadData *load_data, const char *relation_name, const char *file_name)
{
  load_data->relation_name = strdup(relation_name);

  if (file_name[0] == '\'' || file_name[0] == '\"') {
    file_name++;
  }
  char *dup_file_name = strdup(file_name);
  int len = strlen(dup_file_name);
  if (dup_file_name[len - 1] == '\'' || dup_file_name[len - 1] == '\"') {
    dup_file_name[len - 1] = 0;
  }
  load_data->file_name = dup_file_name;
}

void load_data_destroy(LoadData *load_data)
{
  free((char *)load_data->relation_name);
  free((char *)load_data->file_name);
  load_data->relation_name = nullptr;
  load_data->file_name = nullptr;
}

void query_init(Query *query)
{
  query->flag = SCF_ERROR;
  memset(&query->sstr, 0, sizeof(query->sstr));
}

Query *query_create()
{
  Query *query = (Query *)malloc(sizeof(Query));
  if (nullptr == query) {
    LOG_ERROR("Failed to alloc memroy for query. size=%ld", sizeof(Query));
    return nullptr;
  }

  query_init(query);
  return query;
}

void query_reset(Query *query)
{
  switch (query->flag) {
    case SCF_SELECT: {
      selects_destroy(&query->sstr.selection);
    } break;
    case SCF_INSERT: {
      inserts_destroy(&query->sstr.insertion);
    } break;
    case SCF_DELETE: {
      deletes_destroy(&query->sstr.deletion);
    } break;
    case SCF_UPDATE: {
      updates_destroy(&query->sstr.update);
    } break;
    case SCF_CREATE_TABLE: {
      create_table_destroy(&query->sstr.create_table);
    } break;
    case SCF_DROP_TABLE: {
      drop_table_destroy(&query->sstr.drop_table);
    } break;
    case SCF_CREATE_INDEX: {
      create_index_destroy(&query->sstr.create_index);
    } break;
    case SCF_DROP_INDEX: {
      drop_index_destroy(&query->sstr.drop_index);
    } break;
    case SCF_SHOW_INDEX: {
      show_index_destroy(&query->sstr.show_index);
    } break;
    case SCF_SYNC: {

    } break;
    case SCF_SHOW_TABLES:
      break;

    case SCF_DESC_TABLE: {
      desc_table_destroy(&query->sstr.desc_table);
    } break;

    case SCF_LOAD_DATA: {
      load_data_destroy(&query->sstr.load_data);
    } break;
    case SCF_CLOG_SYNC:
    case SCF_BEGIN:
    case SCF_COMMIT:
    case SCF_ROLLBACK:
    case SCF_HELP:
    case SCF_EXIT:
    case SCF_ERROR:
      break;
  }
}

void query_destroy(Query *query)
{
  query_reset(query);
  free(query);
}
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

////////////////////////////////////////////////////////////////////////////////

extern "C" int sql_parse(const char *st, Query *sqls);

RC parse(const char *st, Query *sqln)
{
  int ret = sql_parse(st, sqln);
  if(ret == -10)  {
    LOG_WARN("Date not valid!");
    return INVALID_ARGUMENT;
  } else if (ret == -20) {
    LOG_WARN("aggregation mixed!");
    return INVALID_ARGUMENT;
  }
  if (sqln->flag == SCF_ERROR)
    return SQL_SYNTAX;
  else
    return SUCCESS;
}
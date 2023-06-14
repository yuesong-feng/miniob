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
// Created by WangYunlai on 2022/07/05.
//

#include "sql/expr/tuple_cell.h"
#include "sql/parser/parse_defs.h"
#include "storage/common/field.h"
#include "common/log/log.h"
#include "util/comparator.h"
#include "util/util.h"

void TupleCell::to_string(std::ostream &os) const
{
  switch (attr_type_) {
  case INTS: {
    os << *(int *)data_;
  } break;
  case DATES: {
    int date = *(int *)data_;
    char buf[16] = {0};
    snprintf(buf,sizeof(buf),"%04d-%02d-%02d",date/10000, (date%10000)/100, date%100); // 注意这里月份和天数，不足两位时需要填充0
    os << buf;
  } break;
  case FLOATS: {
    float v = *(float *)data_;
    os << double2string(v);
  } break;
  case TEXTS:
  case CHARS: {
    for (int i = 0; i < length_; i++) {
      if (data_[i] == '\0') {
        break;
      }
      os << data_[i];
    }
  } break;
  default: {
    LOG_WARN("unsupported attr type: %d", attr_type_);
  } break;
  }
}

int TupleCell::compare(const TupleCell &other) const
{
  if (this->attr_type_ == NULLS && other.attr_type_ == NULLS) {
    return 0;
  }
  else if (this->attr_type_ == NULLS){
    return -1;
  }
  else if (other.attr_type_ == NULLS){
    return 1;
  }

  if (this->attr_type_ == other.attr_type_) {
    switch (this->attr_type_) {
    case INTS: 
    case DATES: return compare_int(this->data_, other.data_);
    case FLOATS: return compare_float(this->data_, other.data_);
    case CHARS: return compare_string(this->data_, this->length_, other.data_, other.length_);
    default: {
      LOG_WARN("unsupported type: %d", this->attr_type_);
    }
    }
  } else if (this->attr_type_ == INTS && other.attr_type_ == FLOATS) {
    float this_data = *(int *)data_;
    return compare_float(&this_data, other.data_);
  } else if (this->attr_type_ == FLOATS && other.attr_type_ == INTS) {
    float other_data = *(int *)other.data_;
    return compare_float(data_, &other_data);
  } else if (this->attr_type_ == INTS && other.attr_type_ == CHARS) {
    float this_data = *(int *)data_;
    float other_data = *(float*)chars2floats(other.data_);
    // TODO: 释放内存
    return compare_float(&this_data, &other_data);
  } else if (this->attr_type_ == CHARS && other.attr_type_ == INTS) {
    float this_data = *(float*)chars2floats(data_);
    float other_data = *(int *)other.data_;
    // TODO: 释放内存
    return compare_float(&this_data, &other_data);
  } else if (this->attr_type_ == FLOATS && other.attr_type_ == CHARS) {
    float other_data = *(float *)chars2floats(other.data_);
    // TODO: 释放内存
    return compare_float(this->data_, &other_data);
  } else if (this->attr_type_ == CHARS && other.attr_type_ == FLOATS) {
    float this_data = *(float*)chars2floats(data_);
    // TODO: 释放内存
    return compare_float(&this_data, other.data_);
  } 
  LOG_WARN("not supported");
  return -1; // TODO return rc?
}

bool TupleCell::like(const TupleCell &other) const {
  if (this->attr_type_ != other.attr_type_ || this->attr_type_ != CHARS) {
    LOG_WARN("LIKE only support chars!");
    return false; // TODO return rc?
  }
  const char *str1 = this->data_;
  // int len1 = this->length_;
  int len1 = strlen(str1);
  const char *str2 = other.data_;
  // int len2 = other.length_;
  int len2 = strlen(str2);
  // '%'用于匹配零个到多个任意字符（英文单引号“'”除外），'_'用于匹配一个任意字符（英文单引号“'”除外）。
  
  bool dp[len1+1][len2+1];
  memset(dp, 0, sizeof(dp));
  dp[0][0] = true;
  if(len2>0 && str2[0]=='%'){
    for(int i=0; i<=len1; ++i){
        dp[i][1] = true;
    }
  }
  for(int i=1; i<=len1; ++i){
    for(int j=1; j<=len2; ++j){
      if(str2[j-1]=='%'){
        dp[i][j] = (dp[i][j-1] || dp[i-1][j] || dp[i-1][j-1]);
      }
      else if(str2[j-1]=='_' || str2[j-1]==str1[i-1]){
        dp[i][j] = dp[i-1][j-1];
      }
      else if(isLetter(str2[j-1]) && isLetter(str1[i-1])){ // 不区分大小写
        if(isLowercaseLetter(str2[j-1]) && isUppercaseLetter(str1[i-1]) && toUppercaseLetter(str2[j-1])==str1[i-1]
        || isUppercaseLetter(str2[j-1]) && isLowercaseLetter(str1[i-1]) && toLowercaseLetter(str2[j-1])==str1[i-1]){
          dp[i][j] = dp[i-1][j-1];
        }
      }
    }
  }

  return dp[len1][len2];
}
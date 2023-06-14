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
// Created by wangyunlai on 2021/6/11.
//

#include <string.h>
#include <algorithm>

const double epsilon = 1E-6;

int compare_int(const void *arg1, const void *arg2)
{
  int v1 = *(int *)arg1;
  int v2 = *(int *)arg2;
  return v1 - v2;
}

int compare_int(const void *arg1, const void *arg2, int len)
{
  char* tmp1 = new char[len+1];
  memset(tmp1, 0, sizeof(char)*(len+1));
  memcpy(tmp1, arg1, len);
  char* tmp2 = new char[len+1];
  memset(tmp2, 0, sizeof(char)*(len+1));
  memcpy(tmp2, arg2, len);

  int v1 = *(int *)tmp1;
  int v2 = *(int *)tmp2;
  // TODO: 释放内存
  return v1 - v2;
}

int compare_float(const void *arg1, const void *arg2)
{
  float v1 = *(float *)arg1; 
  float v2 = *(float *)arg2; 
  float cmp = v1 - v2;
  if (cmp > epsilon) {
    return 1;
  }
  if (cmp < -epsilon) {
    return -1;
  }
  return 0;
}

int compare_float(const void *arg1, const void *arg2, int len)
{
  char* tmp1 = new char[len+1];
  memset(tmp1, 0, sizeof(char)*(len+1));
  memcpy(tmp1, arg1, len);
  char* tmp2 = new char[len+1];
  memset(tmp2, 0, sizeof(char)*(len+1));
  memcpy(tmp2, arg2, len);

  float v1 = *(float *)tmp1; 
  float v2 = *(float *)tmp2; 
  float cmp = v1 - v2;
  // TODO: 释放内存
  if (cmp > epsilon) {
    return 1;
  }
  if (cmp < -epsilon) {
    return -1;
  }
  return 0;
}

int compare_float(float arg1, float arg2) {
  float cmp = arg1 - arg2;
  if (cmp > epsilon) {
    return 1;
  }
  if (cmp < -epsilon) {
    return -1;
  }
  return 0;
}

int compare_string(const void *arg1, int arg1_max_length, const void *arg2, int arg2_max_length)
{
  const char *s1 = (const char *)arg1;
  const char *s2 = (const char *)arg2;
  int maxlen = std::min(arg1_max_length, arg2_max_length);
  int result = strncmp(s1, s2, maxlen);
  if (0 != result) {
    return result;
  }

  if (arg1_max_length > maxlen) {
    return s1[maxlen] - 0;
  }

  if (arg2_max_length > maxlen) {
    return 0 - s2[maxlen];
  }
  return 0;
}

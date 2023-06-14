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
// Created by wangyunlai on 2021/6/11
//

#pragma once

int compare_int(const void *arg1, const void *arg2);
int compare_int(const void *arg1, const void *arg2, int len);
int compare_float(const void *arg1, const void *arg2);
int compare_float(float arg1, float arg2);
int compare_float(const void *arg1, const void *arg2, int len);
int compare_string(const void *arg1, int arg1_max_length, const void *arg2, int arg2_max_length);

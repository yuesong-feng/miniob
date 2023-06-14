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
// Created by wangyunlai on 2022/9/28
//

#pragma once

#include <string>

std::string double2string(double v);
int* chars2ints(char* val_chars);
char* ints2chars(int* val_ints);
float* chars2floats(const char* val_chars);
char* floats2chars(float* val_floats);
float* ints2floats(int* val_ints);
int* floats2ints(float* val_floats);

bool isLetter(char c);
bool isLowercaseLetter(char c);
bool isUppercaseLetter(char c);
char toLowercaseLetter(char c);
char toUppercaseLetter(char c);
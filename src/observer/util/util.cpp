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

#include <string.h>
#include <assert.h>
#include "util/util.h"

std::string double2string(double v)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%.2f", v);
  size_t len = strlen(buf);
  while (buf[len - 1] == '0') {
    len--;
      
  }
  if (buf[len - 1] == '.') {
    len--;
  }

  return std::string(buf, len);
}


int* chars2ints(char* val_chars){
  float* val_floats = chars2floats(val_chars);
  int* val_ints = floats2ints(val_floats);
  return val_ints;
}

char* ints2chars(int* val_ints){
  char* val_chars = (char*)malloc(10 * sizeof(char));
  sprintf(val_chars, "%d", *val_ints);
  return val_chars;
}

float* chars2floats(const char* val_chars){
  float* val_floats = new float(0);
  float before_dot = 0, after_dot = 0;
  bool met_dot = false;
  while(*val_chars != 0){
    if(*val_chars >= '0' && *val_chars <= '9'){
      if(!met_dot){
        before_dot = before_dot * 10 + (*val_chars-'0');
      }
      else{
        after_dot = after_dot * 10 + (*val_chars-'0');
      }
      ++val_chars;
    }
    else if(*val_chars == '.' && !met_dot) {
      met_dot = true;
      ++val_chars;
    }
    else{
      break;
    }
  }

  while(after_dot>=1){
    after_dot /= 10;
  }
  *val_floats = before_dot + after_dot;
  return val_floats;
}

char* floats2chars(float* val_floats){
  char* val_chars = (char*)malloc(10 * sizeof(char));
  sprintf(val_chars, "%f", *val_floats);
  // float转为char*后末尾会补0，这里将末尾的0去掉
  bool have_dot = false;
  for(int i=0; i<strlen(val_chars); ++i){
    if(val_chars[i]=='.'){
      have_dot = true;
      break;
    }
  }
  if(have_dot){
    for(int i=strlen(val_chars)-1; i>=0; --i){
      if(val_chars[i]=='0'){
        val_chars[i] = 0;
      }
      else if(val_chars[i]=='.'){
        val_chars[i] = 0;
        break;
      }
      else{
        break;
      }
    }
  }

  return val_chars;
}

float* ints2floats(int* val_ints){
  return new float(*val_ints);
}

int* floats2ints(float* val_floats){
  return new int(*val_floats+0.5);
}

bool isLetter(char c){
  return isLowercaseLetter(c) || isUppercaseLetter(c);
}

bool isLowercaseLetter(char c){
  return c>='a' && c<='z';
}

bool isUppercaseLetter(char c){
  return c>='A' && c<='Z';
}
char toLowercaseLetter(char c){
  assert(isUppercaseLetter(c));
  return c - 'A' + 'a';
}

char toUppercaseLetter(char c){
  assert(isLowercaseLetter(c));
  return c - 'a' + 'A';
}
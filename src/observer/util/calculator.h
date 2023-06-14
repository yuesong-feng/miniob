#pragma once
//Calculator.h: 头文件
#include <stack>
#include <vector>
#include <string>
using namespace std;

//计算器类
class Calculator
{
public:
	Calculator();
	void getFormat();					//表达式自定义标准格式化
	int getPrior(char c);				//获取算术符号优先级
	void getPostfix();					//后缀表达式转换
	void calResult();					//计算后缀表达式
	void calculate();					//计算方法
	double getResult(bool &devided_by_zero_);					//获取结果

	string operatorSym;					//运算符号
	string infix;						//表达式缓存

private:
	vector<string> postfix;				//后缀表达式向量
	stack<char> symStack;				//符号栈
	stack<double> figStack;				//数字栈
	string stdInfix;					//自定义标准格式化表达式
	double result;						//最终计算结果
	bool devided_by_zero_ = false;
};

double calculate(std::string expr, bool &devided_by_zero);

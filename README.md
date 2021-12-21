# 30天自制C++数据库

所有计算机专业的学生都上过数据库课程，但由于太偏理论，一些概念几乎考完试就忘。本教程可作为CS 186的课后补充，在学习过CS 186后，从零开始构建一个简易的关系数据库。

本教程希望读者：
- 熟悉C/C++语言
- 事先粗略学习一遍CS 186: Introduction to Database Systems（不同于服务器开发，数据库开发几乎全靠数据库理论知识，没有数据库理论基础很难看懂相关组件的代码，如B+树的Alt1、Alt2和Alt3，事务的优化，死锁避免等，所以至少事先看一遍CS 186的PPT，开发中遇到再详细了解。）

本教程模仿《30天自制操作系统》，面向零经验的新手，教你在30天内入门数据库开发。本教程更偏向实践，只写代码，不会涉及到背后的数据库基础理论，涉及到的地方会给出相应书籍的具体章节。

C/C++学习的一个难点在于初学时无法做出实际上的东西，没有反馈，程序都在黑乎乎的命令行里运行，不像web开发，可以随时看到自己学习的成果。本教程的代码都放在code文件夹里，每一阶段学习后都可以得到一个可以编译运行的数据库程序，不断迭代开发。

在code文件夹里有每一天的代码文件夹，进入该文件夹，使用`make`命令编译，会生成一个可执行文件，输入命令`./database`就能看到今天的学习成果！

[day01-构建数据库引擎-B+树(Alt2，支持任意类型键值存储)](https://github.com/yuesong-feng/30dayMakeCppDatabase/blob/master/day01-构建数据库引擎-B%2B树(Alt2，支持任意类型键值存储).md)

[day02-B+树插入的辅助函数](https://github.com/yuesong-feng/30dayMakeCppDatabase/blob/main/day02-B+树插入的辅助函数.md)

[day03-支持B+树插入操作](https://github.com/yuesong-feng/30dayMakeCppDatabase/blob/main/day03-支持B+树插入操作.md)

day04-B+树删除的辅助函数

day05-支持B+树删除操作

day06-将索引和数据持久化到磁盘

day07-支持从磁盘读取

### todo list:

实现行锁与表锁

支持事务（ACID）

处理死锁

处理conflict serializable事务

日志系统

定时器

拷贝控制 && 智能指针 && 性能优化-移动语义

SQL解析

性能测试

使用CMake

......
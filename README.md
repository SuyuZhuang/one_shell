
# 写一个SHELL


## 版本功能

1. 用空格来区分参数，不对引号和\做处理
2. 不支持pipes |
3. 不支持 I/O 重定向  < and >
4. 不支持job control
5. 内置方法：
   a. quit 或者 Ctrl-C 可以退出
   b. help 会打印帮助
6. 命令结尾加上 & ，可以后台运行


## 心路历程

起源是学习CSAPP第8章Exceptional Contrl Flow 的时候，看到书中的例子以及课后实验都是写lab，所以想着以这个为目标来学习本章节的内容。
我在c语言方面比较小白，只是知道大致的语法，以及遇到疑问会查一下文档资料，平时只用来写过一两百行的小项目，对于指针、地址、malloc、free方面也还在逐渐熟悉的过程中。
先是照着书中Figure 8.23、8.24和8.25，抄写了一份无Signal版的shell，能够执行成功，但是需要include csapp.h。
这个版本的执行需要的语句：

```
gcc -Og -o prog myshell.c csapp.c -lpthread
```

然后偶然看到有大牛推荐Stephen Brennan的手写shell的教程，教程写得很详细，也是无signal版本的，所以结合了一下csapp中的内容，改了一版。
到目前为止，用到的知识就是读取一行数据，fork() 进程，以及搭配使用execve 或者execvp来在当前进程中加载和运行新的程序，使用waitpid来回收子进程。

再然后，虽然看完了书中signal章节，也跟完了老师的视频课程，但是下笔写信号这块的内容还是觉得有点模糊不清似懂非懂，所以去把csapp的shell lab内容看了一下，简单跟着实现了对Ctrl-C以及子进程SIGCHILD信号的处理。



## 核心步骤

- 初始化 
  - 读取和执行配置文件 
- 解析和执行
  - 循环从文件或者stdin中读取命令并执行 
  - 打印 > 提示符
  - 从stdin读取命令
    - 采用循环读取，动态分配的方式(reallocate)
  - parse
  - execute
    - fork()  调用1次，返回2次。会复制一个子进程，上下文也完全复制。子进程返回0，父进程返回子进程的PID
    - execvp�(args[0], args + 1) 是系统调用 execve() 的变种，不用传环境参数，并且不用写命令全称(如 在execve中查看当前目录文件要使用/bin/ls -al ，而在execvp 中只需要使用ls -al)
    - waitpid(pid, &status, WUNTRACED�) 等待
- terminate
  - 命令执行结束后，释放空间

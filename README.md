# 写一个SHELL

## 背景
简化版的shell, 用空格来区分参数，不对引号和\做处理

从csapp开始，第一部分是无Signal版本的，内置的命令只有quit
```
gcc -Og -o prog myshell.c csapp.c -lpthread
```

后来看了Brennan的教程，在原版的基础上开始修改和拓展

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
- terminate
  - 命令执行结束后，释放空间
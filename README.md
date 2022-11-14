# 写一个SHELL
简化版的shell, 用空格来区分参数，不对引号和\做处理

从csapp开始，第一部分是无Signal版本的，内置的命令只有quit
```
gcc -Og -o prog myshell.c csapp.c -lpthread
```

后来看了Brennan的教程，在原版的基础上开始修改和拓展
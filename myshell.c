#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void init();
void interpret();
void terminate();
char *read_line();
int parse_args(char *line, char **argv);
void execute(char **args, int bg);
int isBuiltinCommand(char **args);
typedef void handler_t(int);
handler_t* Signal(int signum, handler_t *handler);
void sigint_handler(int sig);
void sigchld_handler(int sig);

int main(int argc, char *argv[]) {
    // 1.初始化
    init();

    // 2.解析和执行
    interpret();

    // 3.收尾
    terminate();
    return EXIT_SUCCESS;
}

void terminate() {
}

/*****************
 * 初始化和 Signal handlers
 *****************/

void init() {
    printf("Welcome, this is sush, pid=%d\n", getpid());
    // 将stderr重定向到stdout
    dup2(1, 2);
    // 注册信号Handlers
    Signal(SIGINT, sigint_handler); /* ctrl-c */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
}


unsigned int Sleep(unsigned int secs)
{
    unsigned int rc;

    if ((rc = sleep(secs)) < 0)
        printf("Sleep error\n");
    return rc;
}

/**
 * 接收到kernel 传来的SIGCHLD信号
 * 处理所有变成僵尸进程的子进程
 *
 * @param sig SIGCHLD信号
 */
void sigchld_handler(int sig)
{
    printf("caught! sigchld_handler prepare to reap children who became zombie   sig=%d\n", sig);
    // 因为在waitpid过程中也可能覆盖errno，所有先保存下来，退出handler时再恢复
    int olderrno = errno;
    while (waitpid(-1, NULL, 0) > 0) {
        printf("Handler reaped child\n");
    }

    if (errno != ECHILD) {
        printf("waitpid error");
    }
    // 简单处理
    Sleep(1);
    errno = olderrno;
}


/**
 * 接收到kernel 传来的SIGINT信号
 * 直接退出shell
 *
 * @param sig SIGINT信号
 */
void sigint_handler(int sig)
{
    printf("  caught! sigint_handler  sig=%d\n", sig);
    exit(0);
}


/*****************
 * 解析和执行
 *****************/


#define TOK_BUFSIZE 64
void interpret() {
    char *line;
    char **args;
    int bufsize = TOK_BUFSIZE;


    while (1) {
        printf("sush>");
        line = read_line();
        args = malloc(sizeof(char *) * bufsize);
        int bg = parse_args(line, args);
        execute(args, bg);

        free(line);
        free(args);
    }
}


/**
 * 读取一行
 * @return 一行字符串，会以\n结尾
 */
char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        // 检查是否到达EOF, 即文件末尾
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}


#define TOK_DELIM " \r\t\n\a"
/**
 * tokenizer 并检查最后一个字符是否是 &
 *
 * @param line 输入的一行字符串
 * @param argv 通过空格等分隔符进行tokenizer，得到字符串数组
 * @return 如果最后一个字符是&返回1，否则返回0
 */
int parse_args(char *line, char **argv) {
    int bufsize = sizeof(argv);
    int argc = 0;
    char *token;

    if (!argv) {
        fprintf(stderr, "sush: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        argv[argc++] = token;

        if (argc >= bufsize) {
            bufsize += TOK_BUFSIZE;
            argv = realloc(argv, sizeof(char*) * bufsize);
            if (!argv) {
                fprintf(stderr, "sush: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    argv[argc] = NULL;

    if (argc == 0) {
        return 1;
    }

    // 检查最后一个字符是否是 &
    if (*argv[argc - 1] == '&') {
        argv[--argc] = NULL;
        return 1;
    }
    return 0;
}


/**
 * 包装signal方法，用来给某种信号注册对应的handler
 * 这个方法csapp书中有
 *
 * @param signum 信号对应的数字
 * @param handler 信号处理的handler
 */
handler_t* Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        printf("Signal error");
    return (old_action.sa_handler);
}

/**
 * 执行命令，会先判断是否是内置命令
 *
 * @param args 所有参数， args[0]是命令
 * @param bg 1 是background 命令，0 是foreground命令
 */
void execute(char **args, int bg) {
    pid_t pid;

    if (args[0] == NULL) {
        return;
    }

    // 不是内置的命令，则调用system call
    if (!isBuiltinCommand(args)) {
        pid = fork();
        if (pid == 0) {
            // 子进程
            if (execvp(args[0], args + 1) < 0) {
                printf("sush: %s Command not found.\n", args[0]);
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            printf("sush: %s System error.\n", args[0]);
        } else {
            // 父进程
            if (!bg) {
                // 创建的不是bg运行的子进程，需要父进程等待
                printf("sush: fg, need wait  pid=%d cmd=%s \n", pid,  args[0]);
                int status;
                waitpid(pid, &status, 0);

                if (status < 0) {
                    printf("sush: %s System error.\n", args[0]);
                }
            } else {
                printf("sush: bg, pid=%d cmd=%s \n", pid,  args[0]);

            }
        }


    }
}




/*****************
 * 内置命令
 *****************/

char *builtin_str[] = {
        "exit",
        "help",
        "cd"
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int isBuiltinCommand(char **args) {
    // quit 退出
    if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }
    // help 帮助
    if (strcmp(args[0], "help") == 0) {
        int i;
        printf("SUSH help:\n");
        printf("Type program names and arguments, and hit enter.\n");
        printf("The following are built in:\n");

        for (i = 0; i < num_builtins(); i++) {
            printf("  %s\n", builtin_str[i]);
        }

        printf("Use the man command for information on other programs.\n");
        return 1;
    }
    // cd change directory
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "sush: expected argument to \"cd\"\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("sush");
            }
        }
        return 1;
    }
    return 0;
}

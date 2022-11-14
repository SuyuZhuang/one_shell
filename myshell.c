#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;
void init();
void interpret();
void terminate();
char *read_line();
int parse_args(char *line, char **argv);
void execute(char **args, int bg);

int isBuiltinCommand(char **args);

int checkIsBg(char **args);

int main(int argc, char *argv[]) {
    // 1.初始化
    init();

    // 2.解释和执行
    interpret();

    // 3.收尾
    terminate();
    return EXIT_SUCCESS;
}

void init() {
    printf("Welcome, this is sush, pid=%d\n", getpid());
}
#define TOK_BUFSIZE 64
void interpret() {
    char *line;
    char **args;
    int bufsize = TOK_BUFSIZE;


    while (1) {
        printf(">");
        line = read_line();
        args = malloc(sizeof(char *) * bufsize);
        int bg = parse_args(line, args);
        execute(args, bg);

        free(line);
        free(args);
    }
}

void terminate() {

}

char *read_line() {
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            // 到达EOF, 文件末尾，或者Ctrl-D
            exit(EXIT_SUCCESS);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}


#define TOK_DELIM " \r\t\n\a"
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
                printf("sush: not bg, need wait  pid=%d cmd=%s \n", pid,  args[0]);
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



/*
  List of builtin commands, followed by their corresponding functions.
 */
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

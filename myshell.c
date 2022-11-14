#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;
void init();
void interpret();
void terminate();
char *read_line();
char **parse_args(char *line);
void execute(char **args);

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

}

void interpret() {
    char *line;
    char **args;

    while (1) {
        printf(">");
        line = read_line();
        args = parse_args(line);
        execute(args);

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

#define TOK_BUFSIZE 64
#define TOK_DELIM " \r\t\n\a"
char **parse_args(char *line) {
    int bufsize = TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char *) *bufsize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "sush: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) *bufsize);
            if (!tokens) {
                fprintf(stderr, "sush: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void execute(char **args) {
    pid_t pid;
    pid_t wpid;
    int isBackground;

    if (args[0] == NULL) {
        return;
    }

    isBackground = checkIsBg(args);

    // 不是内置的命令，则调用system call
    if (!isBuiltinCommand(args)) {
        pid = fork();
        if (pid == 0) {
            // 子进程
            if (execve(args[0], args + 1) == -1) {
                printf("sush: %s Command not found.\n", args[0]);
                exit(0);
            }
        } else if (pid < 0) {
            printf("sush: %s System error.\n", args[0]);
        } else {
            // 父进程
            if (!isBackground) {
                // 创建的不是bg运行的子进程，需要父进程等待
                int status;
                waitpid(pid, &status, 0);

                if (status < 0) {
                    printf("sush: %s System error.\n", args[0]);
                } else {
                    printf("sush: pid=%d cmd=%s \n", pid,  args[0]);
                }
            }
        }


    }
    return;
}

int checkIsBg(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }
    return *args[i - 1] == '&';
}

int isBuiltinCommand(char **args) {
    // quit 退出
    if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }
    return 0;
}

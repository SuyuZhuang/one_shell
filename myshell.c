#include "csapp.h"
#define MAXARGS 128

void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    printf("start\n");
    char cmdline[MAXLINE];

    while(1) {
        /* read */
        printf("> ");
    Fgets(cmdline, MAXLINE, stdin);
    if (feof(stdin)) {
        exit(0);
    }

    /* Evaluate */
    eval(cmdline);
    }
    return 0;
}


/* eval - Evaluate a command line */
void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg =  parseline(buf, argv);
    if (argv[0] == NULL) {
        return;
    }
    
    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {
            if (execve(argv[0], argv, environ) <0 ){
                printf("%s : Command not found.\n", argv[0]);
                exit(0);
            }
        }

        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                unix_error("waitfg: waitpid error");
            } else {
                printf("%d %s", pid, cmdline);
            }
        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) {
    if (!strcmp(argv[0], "quit")) {  /* 只识别quit */
        exit(0);
    }
    if (!strcmp(argv[0], "&")) {   /* 忽略& */
        return 1;
    }
    return 0;
}


/* parseline - Parse the command line and build the argv array*/
int parseline(char *buf, char **argv) {
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf)-1] = ' ';
    while (*buf && (*buf == ' ')) {
        buf++;
    }

    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) {
            buf++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        return 1;
    }

    if ((bg = (*argv[argc-1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    return bg;
}



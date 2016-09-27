/** More complex style checker for ejudge */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

const char *const EJUDGE_STYLE_C = "/opt/ejudge/libexec/ejudge/checkers/style_c";
const char *const INDENT = "indent";
char *const INDENT_OPTIONS[] = {
    "indent",
    "--braces-on-if-line",
    "--cuddle-else",
    "--space-after-cast",
    "--indent-label0",
    "--dont-format-comments",
    "--no-space-after-function-call-names",
    "--no-space-after-parentheses",
    "--space-after-for",
    "--space-after-if",
    "--space-after-while",
    "--cuddle-do-while",
    "--no-blank-lines-after-declarations",
    "--no-blank-lines-after-procedures",
    "--no-blank-lines-after-commas",
    "--no-blank-lines-before-block-comments",
    "--leave-optional-blank-lines",
    "--verbose",
    NULL
};

int main(int argc, char **argv) {
    
    printf("%d\n", __LINE__);

    if (argc != 2) {
        fprintf(stderr, "Usage: style_c2 file\n");
        return 1;
    }

    const char *input = argv[1];

    int style_c = fork();
    if (style_c == -1) {
        perror(argv[0]);
        return 1;
    } else if (style_c == 0) {
        execlp(EJUDGE_STYLE_C, EJUDGE_STYLE_C, input, NULL);
        perror(EJUDGE_STYLE_C);
        return 1;
    } else {
        int status;
        if (wait(&status) == -1) {
            perror(argv[0]);
            return 1;
        } else if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                return WEXITSTATUS(status);
            }
        } else {
            fprintf(stderr, "%s: an error occurred when running style_c\n", argv[0]);
            return 1;
        }
    }

    int pp[2];
    if (pipe(pp) == -1) {
        perror(argv[0]);
        return 1;
    }
    int indent = fork();
    if (indent == -1) {
        perror(argv[0]);
        return 1;
    } else if (indent == 0) {   
        int input_fd = open(input, O_RDONLY);
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
        close(pp[0]);
        dup2(pp[1], STDOUT_FILENO);
        close(pp[1]);
        execvp(INDENT, INDENT_OPTIONS);
        perror(INDENT);
        return 1;
    } else {
        close(pp[1]);
        int status;
        if (wait(&status) == -1) {
            perror(argv[0]);
            return 1;
        } else if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                return WEXITSTATUS(status);
            }
            int input_fd = open(input, O_RDONLY);
            char *buf1 = NULL;
            char *buf2 = NULL;
            int retval = 0;

            buf1 = calloc(1024*1024, 32); // NO MORE THAN 32M
            if (buf1 == NULL) {
                perror(argv[0]);
                retval = 1;
                goto END;
            }
            buf2 = calloc(1024*1024, 32);
            if (buf2 == NULL) {
                perror(argv[0]);
                retval = 1;
                goto END;
            }
            ssize_t n1 = read(pp[0], buf1, 1024*1024*32);
            if (n1 == -1) {
                perror(argv[0]);
                retval = 1;
                goto END;
            }
            ssize_t n2 = read(input_fd, buf2, 1024*1024*32);
            if (n2 == -1) {
                perror(argv[0]);
                retval = 1;
                goto END;
            }
            int line = 0;
            int symb = 0;
            int read = 0;
            while (read < n1 && read < n2 && buf1[read] == buf2[read]) {
                if (buf1[read] == '\n') {
                    line++;
                    symb = 0;
                } else {
                    symb++;
                }
                putchar(buf1[read]);
                read++;
            }
            putchar('\n'); 
            if (read == n1 && n1 == n2) {
                // buffers are equal
                retval = 0;
            } else {
                fprintf(stderr, "%d: %d: Style violation\n", line, symb);
                printf("====FORMATTED====\n");
                write(1, buf1 + read, 10);
                putchar('\n');
                printf("====SOURCE====\n");
                write(1, buf2 + read, 10);
                putchar('\n');

                retval = 1;
            }
END:
            free(buf1);
            free(buf2);
            return retval;
        } else {
            fprintf(stderr, "%s: an error occurred when running indent\n", argv[0]);
            return 1;
        }
    }

    return 0; 
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

static char input[2038];

char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(input, 2048, stdin);

    char* cpy = malloc(strlen(input) + 1);
    strcpy(cpy, input);
    cpy[strlen(cpy) - 1] = '\0';

    return cpy;
}

void add_history(char* input);

#else
#include <editline/readline.h>
#include <editline/history.h>

static char input[2048];

int main() {
    puts("Lispy version 0.0.1");
    puts("Press Ctrl+c to exit\n");

    while (1) {
        fputs("Lisp> ", stdout);
        fgets(input, 2048, stdin);
        char* cpy = malloc(strlen(input) + 1);
        strcpy(cpy, input);
        cpy[strlen(input)] = '\0';
        // char* input = readline("lisp> ");
        //
        // add_history(input);
        //
        printf("No you're a input-%s-cpy-%s-input-%ld-cpy-%ld\n", input, cpy, strlen(input), strlen(cpy));

        free(cpy);
    }

    return 0;
}
#endif

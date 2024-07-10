#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpc.h"

#ifdef _WIN32

static char input[2038];

char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(input, 2048, stdin);

    char* cpy = malloc(strlen(input));
    strcpy(cpy, input);
    cpy[strlen(input) - 1] = '\0';

    return cpy;
}

void add_history(char* input);

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main() {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              " \
number  : /-?[0-9]+/ ; \
operator: '+' | '-' | '*' | '/' ; \
expr: <number> | '(' <operator> <expr>+ ')' ; \
lispy: /^/ <operator> <expr>+ /$/ ; \
",
              Number, Operator, Expr, Lispy);

    puts("Lispy version 0.0.1");
    puts("Press Ctrl+c to exit\n");

    while (1) {
        char* input = readline("lisp> ");

        add_history(input);

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}

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

long eval_operation(char* operator, long n1, long n2) {
    if (strcmp(operator, "-") == 0) { return n1 - n2; }
    if (strcmp(operator, "+") == 0) { return n1 + n2; }
    if (strcmp(operator, "*") == 0) { return n1 * n2; }
    if (strcmp(operator, "/") == 0) { return n1 / n2; }
    if (strcmp(operator, "^") == 0) { return pow(n1, n2); }
    if (strcmp(operator, "%") == 0) { return n1 % n2; }
    if (strcmp(operator, "min") == 0) { return n1 < n2 ? n1 : n2; }
    if (strcmp(operator, "max") == 0) { return n1 > n2 ? n1 : n2; }

    return 0;
}

long eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    char* operator = t->children[1]->contents;
    long result = eval(t->children[2]);

    if (! strstr(t->children[3]->tag, "expr")) {
        if (strcmp(operator, "-") == 0) { return -result; }
        if (strcmp(operator, "+") == 0) { return result; }
    }

    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
        result = eval_operation(operator, result, eval(t->children[i]));
    }

    return result;
}

int main() {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              " \
number  : /-?[0-9]+/ ; \
operator: '+' | '-' | '*' | '/' | '^' | '%' | \"min\" | \"max\" ; \
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
            printf("%ld\n", eval(r.output));
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

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

typedef struct lval {
    int type;
    double num;
    int err;
} lval;

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIVIDED_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval lval_num(double x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    if (v.type == LVAL_ERR) {
        if (v.err == LERR_DIVIDED_BY_ZERO) {
            printf("%s", "Err: division by zero.");
        } else if (v.err == LERR_BAD_OP) {
            printf("%s", "Err: invalid operator.");
        } else if (v.err == LERR_BAD_NUM) {
            printf("%s", "Err: invalid number.");
        }
    } else if(v.type == LVAL_NUM) {
        if (ceil(v.num) == (long) v.num) {
            printf("%ld", (long) v.num);
        } else {
            printf("%f", v.num);
        }
    }
}

void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

lval eval_operation(char* operator, lval n1, lval n2) {
    if (n1.type == LVAL_ERR) return n1;
    if (n2.type == LVAL_ERR) return n2;

    if (strcmp(operator, "-") == 0) { return lval_num(n1.num - n2.num); }
    if (strcmp(operator, "+") == 0) { return lval_num(n1.num + n2.num); }
    if (strcmp(operator, "*") == 0) { return lval_num(n1.num * n2.num); }
    if (strcmp(operator, "^") == 0) { return lval_num(pow(n1.num, n2.num)); }
    if (strcmp(operator, "%") == 0) { return lval_num(fmod(n1.num, n2.num)); }
    if (strcmp(operator, "min") == 0) { return lval_num(n1.num < n2.num ? n1.num : n2.num); }
    if (strcmp(operator, "max") == 0) { return lval_num(n1.num > n2.num ? n1.num : n2.num); }
    if (strcmp(operator, "/") == 0) {
        if (n2.num == 0) {
            return lval_err(LERR_DIVIDED_BY_ZERO);
        }
        return lval_num(n1.num / n2.num);
    }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno = 0;
        double x = strtod(t->contents, NULL);
        return errno == ERANGE ? lval_err(LERR_BAD_NUM) : lval_num(x);
    }

    char* operator = t->children[1]->contents;
    lval result = eval(t->children[2]);

    if (! strstr(t->children[3]->tag, "expr")) {
        if (strcmp(operator, "-") == 0) { return lval_num(-result.num); }
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
number  : /-?[0-9.]+/ ; \
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
            lval_println(eval(r.output));
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

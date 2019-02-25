#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static FILE *f;
static int ch;
static unsigned int val;
enum { plus, minus, times, divide, mod, lparen, rparen, number, eof, illegal };
static void SInit(char *filename) {
    ch = EOF;
    f = fopen(filename, "r+t");
    if (f != NULL)
        ch = getc(f);
}
static void Number() {
    val = 0;
    while (('0' <= ch) && (ch <= '9')) {
        val = val * 10 + ch - '0';
        ch = getc(f);
    }
}
static int SGet() {
    register int sym;
    while ((ch != EOF) && (ch <= ' '))
        ch = getc(f);
    switch (ch) {
    case EOF:
        sym = eof;
        break;
    case '+':
        sym = plus;
        ch = getc(f);
        break;
    case '-':
        sym = minus;
        ch = getc(f);
        break;
    case '*':
        sym = times;
        ch = getc(f);
        break;
    case '/':
        sym = divide;
        ch = getc(f);
        break;
    case '%':
        sym = mod;
        ch = getc(f);
        break;
    case '(':
        sym = lparen;
        ch = getc(f);
        break;
    case ')':
        sym = rparen;
        ch = getc(f);
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        sym = number;
        Number();
        break;
    default:
        sym = illegal;
    }
    return sym;
}
static int sym;
static int Expr();
static int Factor() {
    int result;
    assert((sym == number) || (sym == lparen));
    if (sym == number) {
        sym = SGet();
        result = val;
    } else {
        sym = SGet();
        result = Expr();
        assert(sym == rparen);
        sym = SGet();
    }
    return result;
}
static int Term() {
    int result;
    int val1 = Factor();
    result = val1;
    while ((sym == times) || (sym == divide) || (sym == mod)) {
        int Osym = sym;
        sym = SGet();
        int val2 = Factor();
        if (Osym == times)
            result = val1 * val2;
        if (Osym == divide)
            result = val1 / val2;
        if (Osym == mod)
            result = val1 % val2;
        val1 = result;
    }
    return result;
}
static int Expr() {
    if ((sym == minus) || (sym == plus))
        sym = SGet();
    int result;
    int val1 = Term();
    result = val1;
    while ((sym == plus) || (sym == minus)) {
        int Osym = sym;
        sym = SGet();
        int val2 = Term();
        if (Osym == plus)
            result = val1 + val2;
        if (Osym == minus)
            result = val1 - val2;
        val1 = result;
    }
    return result;
}
int main(int argc, char *argv[]) {
    register int result;
    if (argc == 2) {
        SInit(argv[1]);
        sym = SGet();
        int result = Expr();
        assert(sym == eof);
        printf("ans: %d\n", result);
    } else {
        printf("usage: expreval <filename>\n");
    }
    return 0;
}

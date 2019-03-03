#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static FILE *f;
static int ch;
static unsigned int val;
enum {
    var,
    plus,
    minus,
    times,
    divide,
    mod,
    lparen,
    rparen,
    number,
    eof,
    illegal
};
int is_simplified;

typedef struct NodeDesc *Node;
typedef struct NodeDesc {
    char kind;        // plus, minus, times, divide, number
    int val;          // number: value
    Node left, right; // plus, minus, times, divide: children
} NodeDesc;

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
    case 'x':
        sym = var;
        ch = getc(f);
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
static Node Expr();

static Node Factor() {
    register Node result;
    assert((sym == var) || (sym == number) || (sym == lparen));
    if (sym == number || sym == var) {
        result = malloc(sizeof(NodeDesc));
        result->kind = sym;
        result->val = val;
        result->left = NULL;
        result->right = NULL;
        sym = SGet();
    } else {
        sym = SGet();
        result = Expr();
        assert(sym == rparen);
        sym = SGet();
    }
    return result;
}
static Node Term() {
    register Node root, result;
    root = Factor();
    while ((sym == times) || (sym == divide) || (sym == mod)) {
        result = malloc(sizeof(NodeDesc));
        result->kind = sym;
        result->left = root;
        sym = SGet();
        result->right = Factor();
        root = result;
    }
    return root;
}
static Node Expr() {
    register Node root, result;
    root = Term();
    while ((sym == plus) || (sym == minus)) {
        result = malloc(sizeof(NodeDesc));
        result->kind = sym;
        result->left = root;
        sym = SGet();
        result->right = Term();
        root = result;
    }
    return root;
}
static void Print(Node root, int level) {
    register int i;
    if (root != NULL) {
        Print(root->right, level + 1);
        for (i = 0; i < level; i++)
            printf(" ");
        switch (root->kind) {
        case plus:
            printf("+\n");
            break;
        case minus:
            printf("-\n");
            break;
        case times:
            printf("*\n");
            break;
        case divide:
            printf("/\n");
            break;
        case mod:
            printf("%%\n");
            break;
        case number:
            printf("%d\n", root->val);
            break;
        }
        Print(root->left, level + 1);
    }
}
void prefix(Node node) {
    if (node) {
        switch (node->kind) {
        case plus:
            printf("+");
            break;
        case minus:
            printf("-");
            break;
        case times:
            printf("*");
            break;
        case divide:
            printf("/");
            break;
        case mod:
            printf("%%");
            break;
        case number:
            printf("%d", node->val);
            break;
        case var:
            printf("x");
            break;
        default:
            break;
        }
        printf(" ");
        prefix(node->left);
        prefix(node->right);
    }
}
void postfix(Node node) {
    if (node) {
        postfix(node->left);
        postfix(node->right);
        switch (node->kind) {
        case plus:
            printf("+");
            break;
        case minus:
            printf("-");
            break;
        case times:
            printf("*");
            break;
        case divide:
            printf("/");
            break;
        case mod:
            printf("%%");
            break;
        case number:
            printf("%d", node->val);
            break;
        case var:
            printf("x");
            break;
        default:
            break;
        }
        printf(" ");
    }
}
void infix(Node node) {
    if (node) {
        if (node->left != NULL && node->right != NULL) {
            printf("(");
        }
        infix(node->left);
        switch (node->kind) {
        case plus:
            printf("+");
            break;
        case minus:
            printf("-");
            break;
        case times:
            printf("*");
            break;
        case divide:
            printf("/");
            break;
        case mod:
            printf("%%");
            break;
        case number:
            printf("%d", node->val);
            break;
        case var:
            printf("x");
            break;
        default:
            break;
        }
        infix(node->right);
        if (node->left != NULL && node->right != NULL) {
            printf(")");
        }
    }
}
static Node diff(Node root) {
    Node result;
    Node left, right, left2, leftright;

    if (root->kind == number || root->kind == var) {
        result = malloc(sizeof(NodeDesc));
        result->kind = number;

        if (root->kind == number) {
            result->val = 0;
        } else {
            result->val = 1;
        }
        result->left = NULL;
        result->right = NULL;
        return result;
    } else if (root->kind == plus || root->kind == minus) {
        result = malloc(sizeof(NodeDesc));
        result->kind = root->kind;
        result->left = diff(root->left);
        result->right = diff(root->right);
        return result;
    } else if (root->kind == times) {
        result = malloc(sizeof(NodeDesc));
        result->kind = plus;

        left = malloc(sizeof(NodeDesc));
        left->kind = times;
        left->left = diff(root->left);
        left->right = root->right;

        right = malloc(sizeof(NodeDesc));
        right->kind = times;
        right->left = root->left;
        right->right = diff(root->right);

        result->left = left;
        result->right = right;
        return result;
    } else if (root->kind == divide) {
        result = malloc(sizeof(NodeDesc));
        result->kind = divide;

        left = malloc(sizeof(NodeDesc));
        left->kind = minus;

        left2 = malloc(sizeof(NodeDesc));
        left2->kind = times;
        left2->left = root->right;
        left2->right = diff(root->left);

        leftright = malloc(sizeof(NodeDesc));
        leftright->kind = times;
        leftright->left = root->left;
        leftright->right = diff(root->right);

        left->left = left2;
        left->right = leftright;

        right = malloc(sizeof(NodeDesc));
        right->kind = times;
        right->left = root->right;
        right->right = root->right;

        result->left = left;
        result->right = right;

        return result;
    } else {
        printf("Unsupported operator %d\n", root->kind);
    }
}

static int treeEqual(Node L, Node R) {
    if ((L == NULL) && (R == NULL))
        return 1;
    if ((L == NULL) || (R == NULL))
        return 0;
    if (L->kind == number && R->kind == number)
        if (L->val == R->val)
            return 1;
        else
            return 0;
    else if (L->kind == var && R->kind == var)
        return 1;

    if (!treeEqual(L->left, R->left))
        return 0;
    if (!treeEqual(L->right, R->right))
        return 0;
    if (L->kind == R->kind)
        return 1;
    return 0;
}

static Node simplify(Node root) {
    Node result;

    if (root == NULL)
        return root;
    if (root->kind == number || root->kind == var)
        return root;
    if (root->kind == times) {
        // 0*x
        if (root->left->kind == number && root->left->val == 0) {
            is_simplified = 1;
            return root->left;
        }
        // x*0
        if (root->right->kind == number && root->right->val == 0) {
                is_simplified = 1;
                return root->right;
            }
        // 1*x
        if (root->left->kind == number && root->left->val == 1) {
            is_simplified = 1;
            return root->right;
        }
        // x*1
        if (root->right->kind == number && root->right->val == 1) {
            is_simplified = 1;
            return root->left;
        }
    } else if (root->kind == plus) {
        // 0+x
        if (root->left->kind == number && root->left->val == 0) {
            is_simplified = 1;
            return root->right;
        }
        // x+0
        if (root->right->kind == number && root->right->val == 0) {
            is_simplified = 1;
            return root->left;
        }
        // x+x
        if (treeEqual(root->left, root->right) == 1) {
            is_simplified = 1;
            result = malloc(sizeof(NodeDesc));
            result->kind = number;
            result->val = 2;
            result->left = NULL;
            result->right = NULL;
            root->left = result;
            root->kind = times;
            return root;
        }
    } else if (root->kind == minus) {
        // func-func
        if (treeEqual(root->left, root->right) == 1) {
            is_simplified = 1;
            result = malloc(sizeof(NodeDesc));
            result->kind = number;
            result->val = 0;
            result->left = NULL;
            result->right = NULL;
            root = result;
            return root;
        }
    }
    root->left = simplify(root->left);
    root->right = simplify(root->right);
    return root;
}

int main(int argc, char *argv[]) {
    register Node result, diffr, simplifyr;
    if (argc == 2) {
        SInit(argv[1]);
        sym = SGet();
        // Print(Expr(), 0);
        result = Expr();
        assert(sym == eof);

        printf("Prefix: ");
        prefix(result);
        printf("\n");

        printf("Infix: ");
        infix(result);
        printf("\n");

        printf("Postfix: ");
        postfix(result);
        printf("\n");

        printf("Diff's infix\n");
        diffr = diff(result);
        infix(diffr);
        printf("\n");

        simplifyr = diffr;

        is_simplified = 1;

        while (is_simplified) {
            is_simplified = 0;
            simplifyr = simplify(simplifyr);
        }

        printf("Simplified infix: \n");
        infix(simplifyr);
        printf("\n");
        printf("Simplified tree: \n");
        Print(simplifyr, 0);
        printf("\n");

    } else {
        printf("usage: expreval <filename>\n");
    }
    return 0;
}
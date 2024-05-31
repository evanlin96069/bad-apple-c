/*
 * Copyright (c) 2024, evanlin96069
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* p;
char* tk;
int len;
int type;  // op:0 string:1 identifier:2 number:3
int C;     // print color

int next(void) {
    len = 0;
    type = 0;
    char c;

    while ((c = *p)) {
        tk = p++;
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
            // skip whitespace
        } else if (c == '#' || (c == '/' && *p == '/')) {
            // skip macro and comment
            while (*p != '\0' && *p != '\n') {
                p++;
            }
        } else if (c == '/' && *p == '*') {
            // skip multi-line comment
            p++;
            while (*p != '\0') {
                if (*p == '*') {
                    p++;
                    if (*p == '/') {
                        p++;
                        break;
                    }
                }
                p++;
            }
        } else if (c == '"' || c == '\'') {
            // string literal
            type = 1;
            len = 1;
            while (*p != '\0' && *p != c) {
                if (*p == '\\') {
                    len++;
                    p++;
                }
                len++;
                p++;
            }
            len++;
            p++;
            break;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                   c == '_') {
            // identifier
            type = 2;
            len = 1;
            while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
                   (*p >= '0' && *p <= '9') || *p == '_') {
                p++;
                len++;
            }
            break;
        } else if (c >= '0' && c <= '9') {
            // number
            // TODO: parse hex, oct, and float
            type = 3;
            len = 1;
            while ((*p >= '0' && *p <= '9')) {
                p++;
                len++;
            }
            break;
        } else if (c == '=' || c == '*' || c == '/' || c == '%' || c == '^' ||
                   c == '!') {
            // = ==
            // * *=
            // / /=
            // % %=
            // ^ ^=
            // ! !=
            len = 1;
            if (*p == '=') {
                p++;
                len = 2;
            }
            break;
        } else if (c == '-') {
            // - -- -= ->
            len = 1;
            if (*p == c || *p == '=' || *p == '>') {
                p++;
                len = 2;
            }
            break;
        } else if (c == '+' || c == '&' || c == '|') {
            // + ++ +=
            // & && &=
            // | || |=
            len = 1;
            if (*p == c || *p == '=') {
                p++;
                len = 2;
            }
            break;
        } else if (c == '<' || c == '>') {
            // < << <<= <=
            // > >> >>= >=
            len = 1;
            if (*p == c) {
                p++;
                len = 2;
                if (*p == '=') {
                    p++;
                    len = 3;
                }
            } else if (*p == '=') {
                p++;
                len = 2;
            }
            break;
        } else if (c == ';' || c == '~' || c == '?' || c == ':' || c == ',' ||
                   c == '(' || c == ')' || c == '[' || c == ']' || c == '{' ||
                   c == '}' || c == '.') {
            len = 1;
            break;
        }
    }
    return len;
}

char* read_file(const char* path) {
    void* f = fopen(path, "r");
    if (!f) {
        return 0;
    }

    fseek(f, 0, 2);  // SEEK_END
    long sz = ftell(f);
    rewind(f);

    char* buf = malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }

    if (fread(buf, 1, sz, f) != sz) {
        free(buf);
        fclose(f);
        return 0;
    }

    fclose(f);

    buf[sz] = '\0';

    return buf;
}

int c_col = 37;  // current color

const char* kw1[] = {
    "break", "case", "continue", "default", "do",     "else",
    "for",   "goto", "if",       "return",  "switch", "while",
};

const char* kw2[] = {
    "auto",  "char",     "const",  "double",   "enum",     "extern",
    "float", "inline",   "int",    "long",     "register", "restrict",
    "short", "signed",   "sizeof", "static",   "struct",   "typedef",
    "union", "unsigned", "void",   "volatile",
};

// I inlined this function so we can have more tokens to fill up the frame
void print() {
    if (C) {
        int c;
        if (type == 1) {
            // string
            c = 33;  // yellow
        } else if (type == 2) {
            // identifier
            c = 37;  // white

            for (unsigned int i = 0; i < sizeof(kw1) / sizeof(kw1[0]); i++) {
                if (strncmp(tk, kw1[i], len) == 0) {
                    // keyword
                    c = 31;  // red
                    break;
                }
            }

            if (c == 37) {
                for (unsigned int i = 0; i < sizeof(kw1) / sizeof(kw1[0]);
                     i++) {
                    if (strncmp(tk, kw2[i], len) == 0) {
                        // primary type
                        c = 34;  // blue
                        break;
                    }
                }
            }
        } else if (type == 3) {
            // number
            c = 32;  // green
        } else {
            c = 37;  // white
        }

        if (c != c_col) {
            printf("\x1b[%dm", c);
            c_col = c;
        }
    }

    printf("%.*s", len, tk);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        return 1;
    }

    if (argc == 4) {
        C = 1;
    }

    char* src = read_file(argv[1]);
    if (!src) {
        return 1;
    }

    char* img = read_file(argv[2]);
    if (!img) {
        free(src);
        return 1;
    }

    p = src;

    int p_tk = 0;  // previous token type
    int p_ch = 0;  // previous char in the image. 0:space, 1:char

    int w = 0;  // width

    char* curr = img;
    while (*curr) {
        if (*curr == ' ' || *curr == '\n') {
            if (p_ch == 1) {
                // output token
                int l = 0;
                int no_tk = 0;
                while (l < w) {
                    if (!next()) {
                        no_tk = 1;
                        break;
                    }
                    l += len;
                    if (p_tk == 2 && type > 1) {
                        printf(" ");
                    }
                    p_tk = type;

                    // print() begin
                    if (C) {
                        int c;
                        if (type == 1) {
                            // string
                            c = 33;  // yellow
                        } else if (type == 2) {
                            // identifier
                            c = 37;  // white

                            for (unsigned int i = 0;
                                 i < sizeof(kw1) / sizeof(kw1[0]); i++) {
                                if (strncmp(tk, kw1[i], len) == 0) {
                                    // keyword
                                    c = 31;  // red
                                    break;
                                }
                            }

                            if (c == 37) {
                                for (unsigned int i = 0;
                                     i < sizeof(kw2) / sizeof(kw2[0]); i++) {
                                    if (strncmp(tk, kw2[i], len) == 0) {
                                        // primary type
                                        c = 34;  // blue
                                        break;
                                    }
                                }
                            }
                        } else if (type == 3) {
                            // number
                            c = 32;  // green
                        } else {
                            c = 37;  // white
                        }

                        if (c != c_col) {
                            printf("\x1b[%dm", c);
                            c_col = c;
                        }
                    }

                    printf("%.*s", len, tk);
                    // print() end
                }
                if (no_tk == 1) {
                    break;
                }
                w -= l;
            }

            p_ch = 0;
            if (*curr == '\n') {
                w = 0;
                printf("\n");
            } else if (w == 0) {
                printf(" ");
            }

            if (w < 0) {
                w++;
            }
        } else {
            p_ch = 1;
            w++;
        }
        curr++;
    }

    // rest of the tokens
    p_tk = 0;
    while (next()) {
        if (p_tk == 2 && type > 1) {
            printf(" ");
        }
        p_tk = type;

        // print() begin
        if (C) {
            int c;
            if (type == 1) {
                // string
                c = 33;  // yellow
            } else if (type == 2) {
                // identifier
                c = 37;  // white

                for (unsigned int i = 0; i < sizeof(kw1) / sizeof(kw1[0]);
                     i++) {
                    if (strncmp(tk, kw1[i], len) == 0) {
                        // keyword
                        c = 31;  // red
                        break;
                    }
                }

                if (c == 37) {
                    for (unsigned int i = 0; i < sizeof(kw2) / sizeof(kw2[0]);
                         i++) {
                        if (strncmp(tk, kw2[i], len) == 0) {
                            // primary type
                            c = 34;  // blue
                            break;
                        }
                    }
                }
            } else if (type == 3) {
                // number
                c = 32;  // green
            } else {
                c = 37;  // white
            }

            if (c != c_col) {
                printf("\x1b[%dm", c);
                c_col = c;
            }
        }

        printf("%.*s", len, tk);
        // print() end
    }
    if (C) {
        printf("\x1b[0m");
    }

    printf("\n");

    free(src);
    free(img);

    return 0;
}

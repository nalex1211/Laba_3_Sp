#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKEN_LEN 300

typedef enum {
    TK_NUMBER,
    TK_STRING,
    TK_COMMENT,
    TK_RESERVED_WORD,
    TK_OPERATOR,
    TK_PUNCTUATION,
    TK_IDENTIFIER,
    TK_ERROR,
    TK_NONE
} TokenType;

typedef enum {
    STATE_NONE,
    STATE_VAR_DECLARATION,
    STATE_VAL_DECLARATION
} LexerState;

const char *keywords[] = {
        "fun", "var", "val", "if", "else", "for", "while", "String", "Array", "println", "in", "main", "args"
};

bool is_keyword(const char *word) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_punctuation(char ch) {
    const char *punctuations = ",:().{}[];";
    for (int i = 0; i < strlen(punctuations); i++) {
        if (ch == punctuations[i]) {
            return true;
        }
    }
    return false;
}

bool is_operator(char ch) {
    const char *operators = "=+-*/<>";
    for (int i = 0; i < strlen(operators); i++) {
        if (ch == operators[i]) {
            return true;
        }
    }
    return false;
}

void lexer(FILE *input, FILE *output) {
    char ch, buffer[MAX_TOKEN_LEN];
    int buffer_index = 0;
    LexerState state = STATE_NONE;

    while ((ch = fgetc(input)) != EOF) {
        buffer_index = 0;

        if (isspace(ch)) continue;

        if (ch == '/') {
            char next_ch = fgetc(input);
            if (next_ch == '/') {
                while ((ch = fgetc(input)) != '\n' && ch != EOF) {
                    buffer[buffer_index++] = ch;
                }
                buffer[buffer_index] = '\0';
                fprintf(output, "< //%s , COMMENT >\n", buffer);
                continue;
            } else if (next_ch == '*') {
                buffer[buffer_index++] = '*';
                do {
                    ch = fgetc(input);
                    buffer[buffer_index++] = ch;
                    if (ch == '*' && (next_ch = fgetc(input)) == '/') {
                        buffer[buffer_index++] = '/';
                        break;
                    } else {
                        ungetc(next_ch, input);
                    }
                } while (ch != EOF);
                buffer[buffer_index] = '\0';
                fprintf(output, "< /*%s*/ , COMMENT >\n", buffer);
                continue;
            } else {
                ungetc(next_ch, input);
            }
        }

        if (ch == '0') {
            char next_ch = fgetc(input);
            if (next_ch == 'x' || next_ch == 'X') {
                buffer[buffer_index++] = ch;
                buffer[buffer_index++] = next_ch;
                while (isxdigit(ch = fgetc(input))) {
                    buffer[buffer_index++] = ch;
                }
                buffer[buffer_index] = '\0';
                fprintf(output, "< %s , HEXADECIMAL NUMBER >\n", buffer);
                ungetc(ch, input);
                continue;
            } else {
                ungetc(next_ch, input);
            }
        }

        if (isdigit(ch)) {
            do {
                buffer[buffer_index++] = ch;
                ch = fgetc(input);
            } while (isdigit(ch) || ch == '.');
            buffer[buffer_index] = '\0';
            fprintf(output, "< %s , NUMBER >\n", buffer);
            ungetc(ch, input);
            continue;
        }

        if (ch == '"') {
            do {
                buffer[buffer_index++] = ch;
                ch = fgetc(input);
            } while (ch != '"' && ch != EOF && ch != '\n');
            if (ch == '"') {
                buffer[buffer_index++] = ch;
            } else {
                ungetc(ch, input);
                fprintf(output, "< %s , ERROR >\n", buffer);
                continue;
            }
            buffer[buffer_index] = '\0';
            fprintf(output, "< %s , STRING >\n", buffer);
            continue;
        }

        if (isalpha(ch) || ch == '_') {
            do {
                buffer[buffer_index++] = ch;
                ch = fgetc(input);
            } while (isalnum(ch) || ch == '_');
            buffer[buffer_index] = '\0';

            if (is_keyword(buffer)) {
                fprintf(output, "< %s , KEYWORD >\n", buffer);
                if (strcmp(buffer, "var") == 0) {
                    state = STATE_VAR_DECLARATION;
                } else if (strcmp(buffer, "val") == 0) {
                    state = STATE_VAL_DECLARATION;
                } else {
                    state = STATE_NONE;
                }
            } else {
                if (state == STATE_VAR_DECLARATION || state == STATE_VAL_DECLARATION) {
                    fprintf(output, "< %s , IDENTIFIER >\n", buffer);
                    state = STATE_NONE;
                } else {
                    fprintf(output, "< %s , ERROR >\n", buffer);
                }
            }
            ungetc(ch, input);
            continue;
        }

        if (is_punctuation(ch)) {
            fprintf(output, "< %c , PUNCTUATION >\n", ch);
            continue;
        }

        if (is_operator(ch)) {
            buffer[buffer_index++] = ch;
            char next_ch = fgetc(input);
            if ((ch == '<' || ch == '>' || ch == '=' || ch == '!') && next_ch == '=') {
                buffer[buffer_index++] = next_ch;
            } else {
                ungetc(next_ch, input);
            }
            buffer[buffer_index] = '\0';
            fprintf(output, "< %s , OPERATOR >\n", buffer);
            continue;
        }

        fprintf(output, "< %c , ERROR >\n", ch);
    }
}

int main() {
    char filepath[256];
    strcpy(filepath, "input.txt");
    FILE *input = fopen(filepath, "r");

    while (!input) {
        printf("Error opening file %s\n", filepath);
        printf("Enter the full path to the file, or 'n' to exit: ");
        fgets(filepath, sizeof(filepath), stdin);
        size_t len = strlen(filepath);
        if (len > 0 && filepath[len - 1] == '\n') {
            filepath[len - 1] = '\0';
        }
        if (strcmp(filepath, "n") == 0 || strcmp(filepath, "N") == 0) {
            return 1;
        }

        input = fopen(filepath, "r");
    }

    FILE *output = fopen("output.txt", "w");

    lexer(input, output);
    fclose(input);
    fclose(output);
    printf("Program was completed successfully");

    return 0;
}


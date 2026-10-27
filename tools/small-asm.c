#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_LABEL_LENGTH 63
#define MAX_LABELS 256
#define MAX_PROGRAM_BYTES 0x100000u

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    uint32_t address;
} Label;

static Label labels[MAX_LABELS];
static unsigned int label_count;
static int current_pass;
static const char *diagnostic;

/* Input has already been normalized to uppercase ASCII. */
static int valid_label(const char *name) {
    size_t length = strlen(name);
    if (length == 0 || length > MAX_LABEL_LENGTH ||
        !((name[0] >= 'A' && name[0] <= 'Z') || name[0] == '_')) {
        return 0;
    }
    for (size_t i = 1; i < length; i++) {
        if (!((name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= '0' && name[i] <= '9') || name[i] == '_')) {
            return 0;
        }
    }
    return 1;
}

static int find_label(const char *name, uint32_t *address) {
    for (unsigned int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            *address = labels[i].address;
            return 1;
        }
    }
    return 0;
}

static void write_u32_be(FILE *file, uint32_t value) {
    fputc((value >> 24) & 0xFF, file);
    fputc((value >> 16) & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
    fputc(value & 0xFF, file);
}

static void uppercase(char *text) {
    for (; *text != '\0'; text++) {
        *text = toupper((unsigned char)*text);
    }
}

static char *trim(char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }

    char *end = text + strlen(text);

    while (end > text && isspace((unsigned char)end[-1])) {
        end--;
    }

    *end = '\0';
    return text;
}

static int parse_register(const char *text, uint8_t *reg) {
    if (text[0] != 'R' || text[1] < '0' || text[1] > '7' || text[2] != '\0') {
        return 0;
    }

    *reg = (uint8_t)(text[1] - '0');
    return 1;
}

static int parse_memory_register(const char *text, uint8_t *reg) {
    if (strlen(text) != 4 || text[0] != '[' || text[3] != ']') {
        return 0;
    }

    char name[] = {text[1], text[2], '\0'};
    return parse_register(name, reg);
}

static int parse_imm20(const char *text, uint32_t *value) {
    char *end = NULL;

    errno = 0;
    unsigned long parsed = strtoul(text, &end, 0);

    if (errno != 0 || end == text || *end != '\0' || parsed > 0x000FFFFF) {
        return 0;
    }

    *value = (uint32_t)parsed;
    return 1;
}

static int parse_target(const char *text, uint32_t *value) {
    if (parse_imm20(text, value)) {
        return 1;
    }
    if (!valid_label(text)) {
        return 0;
    }
    if (current_pass == 1) {
        *value = 0; /* All definitions will be available on pass 2. */
        return 1;
    }
    if (!find_label(text, value)) {
        diagnostic = "undefined label";
        return 0;
    }
    return 1;
}

/* Normalize syntax, but preserve quoted bytes and punctuation. */
static char *normalize_line(char *line) {
    int quoted = 0;
    char *colon = NULL;
    for (char *p = line; *p != '\0'; p++) {
        if (quoted) {
            if (*p == '\\' && p[1] != '\0') {
                p++;
            } else if (*p == '"') {
                quoted = 0;
            }
        } else if (*p == '"') {
            quoted = 1;
        } else if (*p == ';' || *p == '#') {
            *p = '\0';
            break;
        } else {
            *p = toupper((unsigned char)*p);
            if (*p == ':') {
                if (colon != NULL) {
                    diagnostic = "invalid label definition";
                    return NULL;
                }
                colon = p;
            }
        }
    }
    return line;
}

/* Quoted colons belong to string data, not label definitions. */
static char *prepare_line(char *line, uint32_t address) {
    if (normalize_line(line) == NULL) {
        return NULL;
    }
    char *clean = trim(line);
    char *colon = strchr(clean, ':');
    char *quote = strchr(clean, '"');
    if (quote != NULL && colon != NULL && quote < colon) {
        colon = NULL;
    }
    if (colon != NULL) {
        *colon = '\0';
        char *name = trim(clean);
        uint32_t ignored;
        if (!valid_label(name)) {
            diagnostic = "invalid label definition";
            return NULL;
        }
        if (address > 0xFFFFF) {
            diagnostic = "label address out of range";
            return NULL;
        }
        if (current_pass == 1) {
            if (find_label(name, &ignored)) {
                diagnostic = "duplicate label";
                return NULL;
            }
            if (label_count == MAX_LABELS) {
                diagnostic = "too many labels";
                return NULL;
            }
            strcpy(labels[label_count].name, name);
            labels[label_count++].address = address;
        }
        clean = trim(colon + 1);
    }
    return clean;
}

/* Return a byte sequence for a directive, 0 for an instruction, -1 on error. */
static int assemble_directive(char *line, uint32_t address,
                              unsigned char *bytes, unsigned int *length) {
    if (*line != '.') {
        return 0;
    }
    char *args = line;
    while (*args != '\0' && !isspace((unsigned char)*args)) {
        args++;
    }
    if (*args != '\0') {
        *args++ = '\0';
    }
    args = trim(args);
    *length = 0;
    if (strcmp(line, ".ALIGN") == 0) {
        uint32_t alignment;
        if (!parse_imm20(args, &alignment) || alignment != 4) {
            diagnostic = "invalid alignment";
            return -1;
        }
        *length = (4 - address % 4) % 4;
        memset(bytes, 0, *length);
        return 1;
    }
    if (strcmp(line, ".STRING") != 0) {
        return -1;
    }
    diagnostic = "invalid string";
    if (*args++ != '"') {
        return -1;
    }
    while (*args != '"') {
        unsigned char byte = (unsigned char)*args++;
        if (byte < 0x20 || byte > 0x7E) {
            return -1;
        }
        if (byte == '\\') {
            switch (*args++) {
                case 'n': byte = '\n'; break;
                case 'r': byte = '\r'; break;
                case 't': byte = '\t'; break;
                case '\\': byte = '\\'; break;
                case '"': byte = '"'; break;
                default:
                    diagnostic = "unsupported string escape";
                    return -1;
            }
        }
        bytes[(*length)++] = byte;
    }
    if (*trim(args + 1) != '\0') {
        return -1;
    }
    bytes[(*length)++] = 0;
    return 1;
}

static int assemble_line(char *line, uint32_t *inst) {
    for (char *p = line; *p != '\0'; p++) {
        if (*p == ';' || *p == '#') {
            *p = '\0';
            break;
        }

        if (*p == ',') {
            *p = ' ';
        }
    }

    char *clean = trim(line);

    if (*clean == '\0') {
        return 0;
    }

    uppercase(clean);

    char *mnemonic = strtok(clean, " \t\r\n");

    if (mnemonic == NULL) {
        return 0;
    }

    if (strcmp(mnemonic, "HALT") == 0 || strcmp(mnemonic, "RET") == 0) {
        if (strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        *inst = strcmp(mnemonic, "HALT") == 0 ? 0x01000000 : 0x02000000;
        return 1;
    }

    if (strcmp(mnemonic, "SYSCALL") == 0) {
        char *imm_text = strtok(NULL, " \t\r\n");

        if (imm_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        uint32_t imm;

        if (!parse_imm20(imm_text, &imm)) {
            return -1;
        }

        *inst = 0x60000000 | imm;
        return 1;
    }

    if (strcmp(mnemonic, "MOVI") == 0) {
        char *reg_text = strtok(NULL, " \t\r\n");
        char *imm_text = strtok(NULL, " \t\r\n");

        if (reg_text == NULL || imm_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        uint8_t reg;
        uint32_t imm;

        if (!parse_register(reg_text, &reg) || !parse_target(imm_text, &imm)) {
            return -1;
        }

        *inst = 0x40000000 | ((uint32_t)reg << 20) | imm;
        return 1;
    }

    if (strcmp(mnemonic, "INC") == 0) {
        char *reg_text = strtok(NULL, " \t\r\n");
        uint8_t reg;

        if (reg_text == NULL || strtok(NULL, " \t\r\n") != NULL ||
            !parse_register(reg_text, &reg)) {
            return -1;
        }

        *inst = 0x20000000 | ((uint32_t)reg << 20);
        return 1;
    }

    if (strcmp(mnemonic, "LDB") == 0 || strcmp(mnemonic, "STB") == 0) {
        char *rd_text = strtok(NULL, " \t\r\n");
        char *rs_text = strtok(NULL, " \t\r\n");
        uint8_t rd;
        uint8_t rs;
        int is_load = strcmp(mnemonic, "LDB") == 0;

        if (rd_text == NULL || rs_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        if (is_load) {
            if (!parse_register(rd_text, &rd) || !parse_memory_register(rs_text, &rs)) {
                return -1;
            }
        } else {
            if (!parse_memory_register(rd_text, &rd) || !parse_register(rs_text, &rs)) {
                return -1;
            }
        }

        *inst = (is_load ? 0x30000000 : 0x31000000) |
                ((uint32_t)rd << 20) | ((uint32_t)rs << 16);
        return 1;
    }

    if (strcmp(mnemonic, "CMP") == 0) {
        char *rd_text = strtok(NULL, " \t\r\n");
        char *rs_text = strtok(NULL, " \t\r\n");

        if (rd_text == NULL || rs_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        uint8_t rd;
        uint8_t rs;

        if (!parse_register(rd_text, &rd) || !parse_register(rs_text, &rs)) {
            return -1;
        }

        *inst = 0x24000000 | ((uint32_t)rd << 20) | ((uint32_t)rs << 16);
        return 1;
    }

    if (strcmp(mnemonic, "JUMP") == 0 || strcmp(mnemonic, "JZ") == 0 || strcmp(mnemonic, "JNZ") == 0 || strcmp(mnemonic, "CALLI") == 0) {
        char *imm_text = strtok(NULL, " \t\r\n");

        if (imm_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        uint32_t imm;

        if (!parse_target(imm_text, &imm)) {
            return -1;
        }

        uint32_t op = 0;

        if (strcmp(mnemonic, "JUMP") == 0) {
            op = 8;
        } else if (strcmp(mnemonic, "CALLI") == 0) {
            op = 9;
        } else if (strcmp(mnemonic, "JZ") == 0) {
            op = 10;
        } else {
            op = 11;
        }

        *inst = 0x60000000 | (op << 24) | imm;
        return 1;
    }

    return -1;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s input.asm output.bin\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");

    if (input == NULL) {
        perror(argv[1]);
        return 1;
    }

    FILE *staged = tmpfile();
    if (staged == NULL) {
        perror("temporary output");
        fclose(input);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    unsigned int instruction_count = 0;
    for (current_pass = 1; current_pass <= 2; current_pass++) {
        unsigned int line_number = 0;
        instruction_count = 0;
        uint32_t address = 0;
        while (fgets(line, sizeof(line), input) != NULL) {
            line_number++;
            diagnostic = "unsupported or invalid instruction";
            if (strchr(line, '\n') == NULL && !feof(input)) {
                diagnostic = "line too long";
                goto source_error;
            }
            char *clean = prepare_line(line, address);
            if (clean == NULL) {
                goto source_error;
            }
            unsigned char bytes[MAX_LINE_LENGTH];
            unsigned int length;
            int directive = assemble_directive(clean, address, bytes, &length);
            if (directive < 0) {
                goto source_error;
            }
            if (directive > 0) {
                if (length > MAX_PROGRAM_BYTES - address) {
                    diagnostic = "program too large";
                    goto source_error;
                }
                if (current_pass == 2) {
                    fwrite(bytes, 1, length, staged);
                }
                address += length;
                continue;
            }
            uint32_t inst;
            int result = assemble_line(clean, &inst);
            if (result < 0) {
                goto source_error;
            }
            if (result > 0) {
                if (address % 4 != 0) {
                    diagnostic = "instruction address is not aligned";
                    goto source_error;
                }
                if (4 > MAX_PROGRAM_BYTES - address) {
                    diagnostic = "program too large";
                    goto source_error;
                }
                if (current_pass == 2) {
                    write_u32_be(staged, inst);
                }
                instruction_count++;
                address += 4;
            }
            continue;

source_error:
            fprintf(stderr, "%s:%u: %s\n", argv[1], line_number, diagnostic);
            fclose(staged);
            fclose(input);
            return 1;
        }
        if (ferror(input) || (current_pass == 1 && fseek(input, 0, SEEK_SET) != 0)) {
            perror(argv[1]);
            fclose(staged);
            fclose(input);
            return 1;
        }
    }
    fclose(input);

    if (ferror(staged) || fflush(staged) != 0 || fseek(staged, 0, SEEK_SET) != 0) {
        perror("temporary output");
        fclose(staged);
        return 1;
    }
    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
        perror(argv[2]);
        fclose(staged);
        return 1;
    }
    int byte;
    while ((byte = fgetc(staged)) != EOF) {
        if (fputc(byte, output) == EOF) {
            break;
        }
    }
    int failed = ferror(staged) || ferror(output);
    if (fclose(output) != 0) {
        failed = 1;
    }
    fclose(staged);
    if (failed) {
        fprintf(stderr, "%s: output copy failed\n", argv[2]);
        return 1;
    }
    printf("assembled %u instructions to %s\n", instruction_count, argv[2]);
    return 0;
}

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

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

    if (strcmp(mnemonic, "HALT") == 0) {
        if (strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        *inst = 0x01000000;
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

        if (!parse_register(reg_text, &reg) || !parse_imm20(imm_text, &imm)) {
            return -1;
        }

        *inst = 0x40000000 | ((uint32_t)reg << 20) | imm;
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

    if (strcmp(mnemonic, "JUMP") == 0 || strcmp(mnemonic, "JZ") == 0 || strcmp(mnemonic, "JNZ") == 0) {
        char *imm_text = strtok(NULL, " \t\r\n");

        if (imm_text == NULL || strtok(NULL, " \t\r\n") != NULL) {
            return -1;
        }

        uint32_t imm;

        if (!parse_imm20(imm_text, &imm)) {
            return -1;
        }

        uint32_t op = 0;

        if (strcmp(mnemonic, "JUMP") == 0) {
            op = 8;
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

    FILE *output = fopen(argv[2], "wb");

    if (output == NULL) {
        perror(argv[2]);
        fclose(input);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    unsigned int line_number = 0;
    unsigned int instruction_count = 0;

    while (fgets(line, sizeof(line), input) != NULL) {
        line_number++;

        if (strchr(line, '\n') == NULL && !feof(input)) {
            fprintf(stderr, "%s:%u: line too long\n", argv[1], line_number);
            fclose(output);
            fclose(input);
            return 1;
        }

        uint32_t inst;
        int result = assemble_line(line, &inst);

        if (result < 0) {
            fprintf(stderr, "%s:%u: unsupported or invalid instruction\n", argv[1], line_number);
            fclose(output);
            fclose(input);
            return 1;
        }

        if (result > 0) {
            write_u32_be(output, inst);
            instruction_count++;
        }
    }

    if (ferror(input)) {
        perror(argv[1]);
        fclose(output);
        fclose(input);
        return 1;
    }

    if (ferror(output)) {
        perror(argv[2]);
        fclose(output);
        fclose(input);
        return 1;
    }

    fclose(output);
    fclose(input);

    printf("assembled %u instructions to %s\n", instruction_count, argv[2]);
    return 0;
}

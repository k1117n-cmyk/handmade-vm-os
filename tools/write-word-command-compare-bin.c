#include <stdint.h>
#include <stdio.h>

#define MAX_INSTS 256

#define PROMPT_TEXT 0x300
#define HELP_TEXT 0x340
#define GOODBYE_TEXT 0x3A0
#define UNKNOWN_TEXT 0x3E0
#define HELP_WORD 0x420
#define QUIT_WORD 0x430
#define INPUT_BUFFER 0x500
#define INPUT_LIMIT 0x53F

static void write_u32_be(FILE *file, uint32_t value) {
    fputc((value >> 24) & 0xFF, file);
    fputc((value >> 16) & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
    fputc(value & 0xFF, file);
}

static int write_string_at(FILE *file, long address, const char *text) {
    if (fseek(file, address, SEEK_SET) != 0) {
        return 0;
    }

    while (*text != '\0') {
        fputc((unsigned char)*text, file);
        text++;
    }

    fputc(0x00, file);
    return 1;
}

static uint32_t address_of(size_t index) {
    return (uint32_t)(index * 4);
}

static size_t emit(uint32_t *program, size_t *count, uint32_t inst) {
    size_t index = *count;

    if (*count >= MAX_INSTS) {
        return MAX_INSTS;
    }

    program[*count] = inst;
    *count += 1;
    return index;
}

static size_t emit_movi(uint32_t *program, size_t *count, uint8_t reg, uint32_t imm) {
    return emit(program, count, 0x40000000 | ((uint32_t)reg << 20) | (imm & 0x000FFFFF));
}

static size_t emit_ldb(uint32_t *program, size_t *count, uint8_t rd, uint8_t rs) {
    return emit(program, count, 0x30000000 | ((uint32_t)rd << 20) | ((uint32_t)rs << 16));
}

static size_t emit_stb(uint32_t *program, size_t *count, uint8_t rd, uint8_t rs) {
    return emit(program, count, 0x31000000 | ((uint32_t)rd << 20) | ((uint32_t)rs << 16));
}

static size_t emit_inc(uint32_t *program, size_t *count, uint8_t reg) {
    return emit(program, count, 0x20000000 | ((uint32_t)reg << 20));
}

static size_t emit_cmp(uint32_t *program, size_t *count, uint8_t rd, uint8_t rs) {
    return emit(program, count, 0x24000000 | ((uint32_t)rd << 20) | ((uint32_t)rs << 16));
}

static size_t emit_syscall(uint32_t *program, size_t *count, uint32_t imm) {
    return emit(program, count, 0x60000000 | (imm & 0x000FFFFF));
}

static size_t emit_jump_placeholder(uint32_t *program, size_t *count, uint32_t opcode) {
    return emit(program, count, opcode);
}

static void patch_jump(uint32_t *program, size_t index, uint32_t opcode, uint32_t address) {
    program[index] = opcode | (address & 0x000FFFFF);
}

int main(int argc, char **argv) {
    const char *path = "programs/word-command-compare.bin";
    uint32_t program[MAX_INSTS];
    size_t count = 0;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [output.bin]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        path = argv[1];
    }

    emit_movi(program, &count, 0, PROMPT_TEXT);       // MOVI R0, prompt
    emit_syscall(program, &count, 1);                 // SYSCALL 1
    size_t call_read_line = emit_jump_placeholder(program, &count, 0x69000000);

    emit_movi(program, &count, 1, INPUT_BUFFER);
    emit_movi(program, &count, 2, HELP_WORD);
    size_t call_compare_help = emit_jump_placeholder(program, &count, 0x69000000);
    emit_movi(program, &count, 3, 1);
    emit_cmp(program, &count, 0, 3);
    size_t jump_do_help = emit_jump_placeholder(program, &count, 0x6A000000);

    // string_equal advances both pointers, so reset them for the second call.
    emit_movi(program, &count, 1, INPUT_BUFFER);
    emit_movi(program, &count, 2, QUIT_WORD);
    size_t call_compare_quit = emit_jump_placeholder(program, &count, 0x69000000);
    emit_movi(program, &count, 3, 1);
    emit_cmp(program, &count, 0, 3);
    size_t jump_do_quit = emit_jump_placeholder(program, &count, 0x6A000000);
    size_t jump_unknown = emit_jump_placeholder(program, &count, 0x68000000);

    uint32_t do_help = address_of(count);
    emit_movi(program, &count, 0, HELP_TEXT);         // MOVI R0, help text
    emit_syscall(program, &count, 1);                 // SYSCALL 1
    emit(program, &count, 0x01000000);                // HALT

    uint32_t do_quit = address_of(count);
    emit_movi(program, &count, 0, GOODBYE_TEXT);      // MOVI R0, goodbye text
    emit_syscall(program, &count, 1);                 // SYSCALL 1
    emit(program, &count, 0x01000000);                // HALT

    uint32_t do_unknown = address_of(count);
    emit_movi(program, &count, 0, UNKNOWN_TEXT);      // MOVI R0, unknown text
    emit_syscall(program, &count, 1);                 // SYSCALL 1
    emit(program, &count, 0x01000000);                // HALT

    // R0/R1/R2 are scratch registers. On return, memory[INPUT_BUFFER]
    // contains up to 63 bytes followed by 0; the caller starts comparing it.
    uint32_t read_line = address_of(count);
    emit_movi(program, &count, 1, INPUT_BUFFER);      // MOVI R1, input buffer
    uint32_t read_loop = address_of(count);
    emit_syscall(program, &count, 2);                 // SYSCALL 2
    emit_movi(program, &count, 2, 10);                // MOVI R2, LF
    emit_cmp(program, &count, 0, 2);                  // CMP R0, R2
    size_t jump_finish_lf = emit_jump_placeholder(program, &count, 0x6A000000);
    emit_movi(program, &count, 2, 13);                // MOVI R2, CR
    emit_cmp(program, &count, 0, 2);                  // CMP R0, R2
    size_t jump_finish_cr = emit_jump_placeholder(program, &count, 0x6A000000);
    emit_stb(program, &count, 1, 0);                  // STB [R1], R0
    emit_inc(program, &count, 1);                     // INC R1
    emit_movi(program, &count, 2, INPUT_LIMIT);       // MOVI R2, input limit
    emit_cmp(program, &count, 1, 2);                  // CMP R1, R2
    size_t jump_finish_limit = emit_jump_placeholder(program, &count, 0x6A000000);
    emit(program, &count, 0x68000000 | read_loop);    // JUMP read_loop
    uint32_t finish_input = address_of(count);
    emit_movi(program, &count, 0, 0);                 // MOVI R0, 0
    emit_stb(program, &count, 1, 0);                  // STB [R1], R0
    emit(program, &count, 0x02000000);                // RET

    // R1/R2: zero-terminated strings; R0: 1 if equal, otherwise 0.
    // Clobbers R0..R4 and the zero flag. Does not write either string.
    uint32_t string_equal = address_of(count);
    emit_movi(program, &count, 4, 0);
    uint32_t compare_loop = address_of(count);
    emit_ldb(program, &count, 0, 1);
    emit_ldb(program, &count, 3, 2);
    emit_cmp(program, &count, 0, 3);
    size_t jump_not_equal = emit_jump_placeholder(program, &count, 0x6B000000);
    emit_cmp(program, &count, 0, 4);
    size_t jump_equal = emit_jump_placeholder(program, &count, 0x6A000000);
    emit_inc(program, &count, 1);
    emit_inc(program, &count, 2);
    emit(program, &count, 0x68000000 | compare_loop);
    uint32_t equal = address_of(count);
    emit_movi(program, &count, 0, 1);
    emit(program, &count, 0x02000000);
    uint32_t not_equal = address_of(count);
    emit_movi(program, &count, 0, 0);
    emit(program, &count, 0x02000000);

    if (count >= MAX_INSTS || address_of(count) > PROMPT_TEXT) {
        fprintf(stderr, "program overlaps prompt text or is too large\n");
        return 1;
    }

    patch_jump(program, call_read_line, 0x69000000, read_line);
    patch_jump(program, jump_finish_lf, 0x6A000000, finish_input);
    patch_jump(program, jump_finish_cr, 0x6A000000, finish_input);
    patch_jump(program, jump_finish_limit, 0x6A000000, finish_input);
    patch_jump(program, call_compare_help, 0x69000000, string_equal);
    patch_jump(program, call_compare_quit, 0x69000000, string_equal);
    patch_jump(program, jump_do_help, 0x6A000000, do_help);
    patch_jump(program, jump_do_quit, 0x6A000000, do_quit);
    patch_jump(program, jump_unknown, 0x68000000, do_unknown);
    patch_jump(program, jump_not_equal, 0x6B000000, not_equal);
    patch_jump(program, jump_equal, 0x6A000000, equal);

    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        perror(path);
        return 1;
    }

    for (size_t i = 0; i < count; i++) {
        write_u32_be(file, program[i]);
    }

    if (!write_string_at(file, PROMPT_TEXT, "Command> ")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, HELP_TEXT, "Commands:\nhelp: show help\nquit: quit\n")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, GOODBYE_TEXT, "Goodbye from Handmade VM\n")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, UNKNOWN_TEXT, "Unknown command\n")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, HELP_WORD, "help") ||
        !write_string_at(file, QUIT_WORD, "quit")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (ferror(file)) {
        perror(path);
        fclose(file);
        return 1;
    }

    fclose(file);
    printf("wrote %s\n", path);
    return 0;
}

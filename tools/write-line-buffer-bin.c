#include <stdint.h>
#include <stdio.h>

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

int main(int argc, char **argv) {
    const char *path = "programs/line-buffer.bin";

    if (argc > 2) {
        fprintf(stderr, "usage: %s [output.bin]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        path = argv[1];
    }

    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        perror(path);
        return 1;
    }

    write_u32_be(file, 0x40000100);  // MOVI R0, 0x100
    write_u32_be(file, 0x60000001);  // SYSCALL 1
    write_u32_be(file, 0x40100180);  // MOVI R1, 0x180
    write_u32_be(file, 0x60000002);  // SYSCALL 2
    write_u32_be(file, 0x4020000A);  // MOVI R2, 10
    write_u32_be(file, 0x24020000);  // CMP R0, R2
    write_u32_be(file, 0x6A000040);  // JZ 0x40
    write_u32_be(file, 0x4020000D);  // MOVI R2, 13
    write_u32_be(file, 0x24020000);  // CMP R0, R2
    write_u32_be(file, 0x6A000040);  // JZ 0x40
    write_u32_be(file, 0x31100000);  // STB [R1], R0
    write_u32_be(file, 0x20100000);  // INC R1
    write_u32_be(file, 0x402001BF);  // MOVI R2, 0x1BF
    write_u32_be(file, 0x24120000);  // CMP R1, R2
    write_u32_be(file, 0x6A000040);  // JZ 0x40
    write_u32_be(file, 0x6800000C);  // JUMP 0x0C
    write_u32_be(file, 0x40000000);  // MOVI R0, 0
    write_u32_be(file, 0x31100000);  // STB [R1], R0
    write_u32_be(file, 0x40000140);  // MOVI R0, 0x140
    write_u32_be(file, 0x60000001);  // SYSCALL 1
    write_u32_be(file, 0x40000180);  // MOVI R0, 0x180
    write_u32_be(file, 0x60000001);  // SYSCALL 1
    write_u32_be(file, 0x4000000A);  // MOVI R0, 10
    write_u32_be(file, 0x60000003);  // SYSCALL 3
    write_u32_be(file, 0x01000000);  // HALT

    if (!write_string_at(file, 0x100, "Type a word, then Enter\n>")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, 0x140, "You typed: ")) {
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

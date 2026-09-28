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
    const char *path = "programs/boot-message.bin";

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
    write_u32_be(file, 0x4000003E);  // MOVI R0, 62
    write_u32_be(file, 0x60000003);  // SYSCALL 3
    write_u32_be(file, 0x60000002);  // SYSCALL 2
    write_u32_be(file, 0x40100068);  // MOVI R1, 104
    write_u32_be(file, 0x24010000);  // CMP R0, R1
    write_u32_be(file, 0x6A000064);  // JZ 0x64
    write_u32_be(file, 0x6B000024);  // JNZ 0x24
    write_u32_be(file, 0x40100071);  // MOVI R1, 113
    write_u32_be(file, 0x24010000);  // CMP R0, R1
    write_u32_be(file, 0x6A000070);  // JZ 0x70
    write_u32_be(file, 0x6B000034);  // JNZ 0x34
    write_u32_be(file, 0x4010000A);  // MOVI R1, 10
    write_u32_be(file, 0x24010000);  // CMP R0, R1
    write_u32_be(file, 0x6A000008);  // JZ 0x08
    write_u32_be(file, 0x6B000044);  // JNZ 0x44
    write_u32_be(file, 0x4010000D);  // MOVI R1, 13
    write_u32_be(file, 0x24010000);  // CMP R0, R1
    write_u32_be(file, 0x6A000008);  // JZ 0x08
    write_u32_be(file, 0x6B000054);  // JNZ 0x54
    write_u32_be(file, 0x40100020);  // MOVI R1, 32
    write_u32_be(file, 0x24010000);  // CMP R0, R1
    write_u32_be(file, 0x6A000008);  // JZ 0x08
    write_u32_be(file, 0x6B00007C);  // JNZ 0x7C
    write_u32_be(file, 0x40000140);  // MOVI R0, 0x140
    write_u32_be(file, 0x60000001);  // SYSCALL 1
    write_u32_be(file, 0x68000008);  // JUMP 0x08
    write_u32_be(file, 0x40000180);  // MOVI R0, 0x180
    write_u32_be(file, 0x60000001);  // SYSCALL 1
    write_u32_be(file, 0x01000000);  // HALT
    write_u32_be(file, 0x4000003F);  // MOVI R0, 63
    write_u32_be(file, 0x60000000);  // SYSCALL 0
    write_u32_be(file, 0x68000008);  // JUMP 0x08

    if (!write_string_at(file, 0x100, "Welcome to Handmade VM\n")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, 0x140, "Commands:\nh: help\nq: quit\n")) {
        perror(path);
        fclose(file);
        return 1;
    }

    if (!write_string_at(file, 0x180, "Goodbye from Handmade VM\n")) {
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

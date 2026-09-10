#include <stdint.h>
#include <stdio.h>

static void write_u32_be(FILE *file, uint32_t value) {
    fputc((value >> 24) & 0xFF, file);
    fputc((value >> 16) & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
    fputc(value & 0xFF, file);
}

int main(int argc, char **argv) {
    const char *path = "programs/hello.bin";

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

    write_u32_be(file, 0x40000041);  // MOVI R0, 65
    write_u32_be(file, 0x60000000);  // SYSCALL 0
    write_u32_be(file, 0x01000000);  // HALT

    if (ferror(file)) {
        perror(path);
        fclose(file);
        return 1;
    }

    fclose(file);
    printf("wrote %s\n", path);
    return 0;
}

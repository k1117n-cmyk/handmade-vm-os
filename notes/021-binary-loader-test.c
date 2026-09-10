#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MEMORY_SIZE (1024 * 1024)

static uint32_t read_u32_be(const uint8_t *memory, uint32_t address) {
    return ((uint32_t)memory[address] << 24) |
           ((uint32_t)memory[address + 1] << 16) |
           ((uint32_t)memory[address + 2] << 8) |
           ((uint32_t)memory[address + 3]);
}

static bool load_program_file(uint8_t *memory, const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        perror(path);
        return false;
    }

    size_t bytes_read = fread(memory, 1, MEMORY_SIZE, file);

    if (ferror(file)) {
        perror(path);
        fclose(file);
        return false;
    }

    if (bytes_read == MEMORY_SIZE && fgetc(file) != EOF) {
        fprintf(stderr, "program too large: %s\n", path);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static bool write_test_program(const char *path) {
    const uint8_t program[] = {
        0x40, 0x00, 0x00, 0x41,  // MOVI R0, 65
        0x60, 0x00, 0x00, 0x00,  // SYSCALL 0
        0x01, 0x00, 0x00, 0x00   // HALT
    };

    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        perror(path);
        return false;
    }

    size_t bytes_written = fwrite(program, 1, sizeof(program), file);

    if (bytes_written != sizeof(program) || ferror(file)) {
        perror(path);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

int main(void) {
    const char *path = "/tmp/021-program.bin";
    uint8_t memory[MEMORY_SIZE] = {0};

    if (!write_test_program(path)) {
        return 1;
    }

    if (!load_program_file(memory, path)) {
        return 1;
    }

    uint32_t inst0 = read_u32_be(memory, 0x00000000);
    uint32_t inst1 = read_u32_be(memory, 0x00000004);
    uint32_t inst2 = read_u32_be(memory, 0x00000008);

    printf("inst0=0x%08X\n", inst0);
    printf("inst1=0x%08X\n", inst1);
    printf("inst2=0x%08X\n", inst2);

    if (inst0 == 0x40000041 &&
        inst1 == 0x60000000 &&
        inst2 == 0x01000000) {
        printf("binary loader test passed.\n");
        return 0;
    }

    printf("binary loader test failed.\n");
    return 1;
}

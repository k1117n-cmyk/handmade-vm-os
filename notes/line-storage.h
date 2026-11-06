#ifndef HANDMADE_LINE_STORAGE_H
#define HANDMADE_LINE_STORAGE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Fixed host file, with a 64-byte VM buffer and at most 63 content bytes. */
static uint32_t save_stored_line(const uint8_t *memory, size_t memory_size,
                                 uint32_t address) {
    if (memory_size < 64 || address > memory_size - 64) {
        return 1;
    }

    size_t length = 0;
    while (length < 64 && memory[address + length] != 0) {
        if (memory[address + length] == '\r' || memory[address + length] == '\n') {
            return 1;
        }
        length++;
    }
    if (length == 64) {
        return 1;
    }

    FILE *file = fopen("line.txt", "wb");
    if (file == NULL) {
        return 1;
    }
    size_t written = fwrite(memory + address, 1, length, file);
    int close_result = fclose(file);
    return written == length && close_result == 0 ? 0 : 1;
}

static uint32_t load_stored_line(uint8_t *memory, size_t memory_size,
                                 uint32_t address) {
    if (memory_size < 64 || address > memory_size - 64) {
        return 1;
    }

    FILE *file = fopen("line.txt", "rb");
    if (file == NULL) {
        return 1;
    }
    uint8_t content[64];
    size_t length = fread(content, 1, sizeof(content), file);
    int read_failed = ferror(file);
    int at_end = feof(file);
    int close_result = fclose(file);
    if (read_failed || !at_end || close_result != 0 || length == 64) {
        return 1;
    }
    for (size_t i = 0; i < length; i++) {
        if (content[i] == 0 || content[i] == '\r' || content[i] == '\n') {
            return 1;
        }
    }

    memcpy(memory + address, content, length);
    memory[address + length] = 0;
    return 0;
}

#endif

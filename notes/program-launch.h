#ifndef PROGRAM_LAUNCH_H
#define PROGRAM_LAUNCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static bool load_child_program(unsigned char *memory, size_t capacity) {
    FILE *file = fopen("programs/run-child.bin", "rb");
    if (file == NULL) {
        return false;
    }

    size_t length = fread(memory, 1, capacity, file);
    bool valid = length >= 4 && !ferror(file);
    if (valid && length == capacity) {
        int extra = fgetc(file);
        valid = extra == EOF && !ferror(file);
    }
    if (fclose(file) != 0) {
        valid = false;
    }
    return valid;
}

#endif

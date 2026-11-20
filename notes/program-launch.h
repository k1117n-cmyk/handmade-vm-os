#ifndef PROGRAM_LAUNCH_H
#define PROGRAM_LAUNCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static inline bool load_fixed_program(unsigned char *memory, size_t capacity,
                                      const char *path) {
    FILE *file = fopen(path, "rb");
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

static inline bool load_child_program(unsigned char *memory, size_t capacity) {
    return load_fixed_program(memory, capacity, "programs/run-child.bin");
}

static inline bool load_editor_program(unsigned char *memory, size_t capacity) {
    return load_fixed_program(memory, capacity, "programs/editor.bin");
}

#endif

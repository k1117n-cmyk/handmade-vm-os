/* Run first as an isolated component, then with -DSTORAGE_VM_TEST. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum Fault { NONE, OPEN_FAIL, READ_FAIL, WRITE_FAIL, CLOSE_FAIL };
static enum Fault fault;
static unsigned opens, opened, closes;

static FILE *test_fopen(const char *name, const char *mode) {
    opens++;
    FILE *file = fault == OPEN_FAIL ? NULL : fopen(name, mode);
    if (file != NULL) opened++;
    return file;
}
static size_t test_fread(void *ptr, size_t size, size_t count, FILE *file) {
    return fread(ptr, size, fault == READ_FAIL ? 1 : count, file);
}
static int test_ferror(FILE *file) {
    return fault == READ_FAIL || ferror(file);
}
static size_t test_fwrite(const void *ptr, size_t size, size_t count, FILE *file) {
    return fwrite(ptr, size, fault == WRITE_FAIL && count ? count - 1 : count, file);
}
static int test_fclose(FILE *file) {
    closes++;
    int result = fclose(file);
    return fault == CLOSE_FAIL ? EOF : result;
}

#define fopen test_fopen
#define fread test_fread
#define ferror test_ferror
#define fwrite test_fwrite
#define fclose test_fclose
#ifdef STORAGE_VM_TEST
#define main vm_program_main
#include "vm.c"
#undef main
#else
#include "line-storage.h"
#define MEMORY_SIZE (1024 * 1024)
#endif
#undef fopen
#undef fread
#undef ferror
#undef fwrite
#undef fclose

static uint8_t memory[MEMORY_SIZE], expected[MEMORY_SIZE];
static unsigned count;

static void put_file(const uint8_t *data, size_t length) {
    FILE *file = fopen("line.txt", "wb");
    assert(file != NULL);
    assert(fwrite(data, 1, length, file) == length);
    assert(fclose(file) == 0);
}

static void check_file(const uint8_t *data, size_t length) {
    uint8_t actual[129];
    FILE *file = fopen("line.txt", "rb");
    assert(file != NULL);
    size_t n = fread(actual, 1, sizeof(actual), file);
    assert(!ferror(file) && feof(file));
    assert(fclose(file) == 0);
    assert(n == length && memcmp(actual, data, length) == 0);
}

static void call(unsigned syscall, uint32_t address, uint32_t result) {
    opens = opened = closes = 0;
#ifdef STORAGE_VM_TEST
    static VM vm;
    init_vm(&vm);
    memcpy(vm.memory, memory, MEMORY_SIZE);
    for (unsigned i = 1; i < 8; i++) vm.regs[i] = 0xA0 + i;
    vm.regs[0] = address;
    vm.sp = MEMORY_SIZE - 16;
    vm.zero_flag = count % 2 != 0;
    /* Fetch from a location outside the tested buffer, then restore its bytes. */
    uint32_t code_address = address == 0 ? 0x100 : 0;
    uint32_t original = read_u32_be(vm.memory, code_address);
    write_inst(&vm, code_address, 0x60000000 | syscall);
    vm.pc = code_address;
    uint32_t inst;
    assert(fetch(&vm, &inst));
    write_u32_be(vm.memory, code_address, original);
    execute(&vm, decode(inst));
    assert(vm.regs[0] == result);
    for (unsigned i = 1; i < 8; i++) assert(vm.regs[i] == 0xA0 + i);
    assert(vm.zero_flag == (count % 2 != 0));
    assert(vm.sp == MEMORY_SIZE - 16 && vm.pc == code_address + 4 && vm.running);
    memcpy(memory, vm.memory, MEMORY_SIZE);
#else
    assert((syscall == 4 ? save_stored_line(memory, MEMORY_SIZE, address)
                        : load_stored_line(memory, MEMORY_SIZE, address)) == result);
#endif
    assert(memcmp(memory, expected, MEMORY_SIZE) == 0);
    assert(closes == opened);
    count++;
}

static void save_case(const char *name, const uint8_t *data, size_t length,
                      uint32_t address, enum Fault failure, uint32_t result,
                      size_t file_length) {
    static const uint8_t old[] = "old file";
    fault = NONE;
    put_file(old, sizeof(old) - 1);
    memset(memory, 0xA5, sizeof(memory));
    if (address <= MEMORY_SIZE - 64) memcpy(memory + address, data, length);
    memcpy(expected, memory, sizeof(memory));
    fault = failure;
    call(4, address, result);
    if (file_length == (size_t)-1) {
        assert(opens == (failure == OPEN_FAIL ? 1u : 0u));
        check_file(old, sizeof(old) - 1);
    } else {
        check_file(data, file_length);
    }
    printf("save: %s OK\n", name);
}

static void load_case(const char *name, const uint8_t *data, size_t length,
                      uint32_t address, enum Fault failure, uint32_t result) {
    fault = NONE;
    put_file(data, length);
    memset(memory, 0xA5, sizeof(memory));
    memcpy(expected, memory, sizeof(memory));
    if (result == 0) {
        memcpy(expected + address, data, length);
        expected[address + length] = 0;
    }
    fault = failure;
    call(5, address, result);
    if (address > MEMORY_SIZE - 64) assert(opens == 0);
    check_file(data, length);
    printf("load: %s OK\n", name);
}

int main(void) {
    char directory[] = "/tmp/handmade-storage-XXXXXX";
    assert(mkdtemp(directory) != NULL);
    assert(chdir(directory) == 0);
    uint8_t max[64], over[65], huge[128];
    memset(max, 'x', 63); max[63] = 0;
    memset(over, 'y', sizeof(over));
    memset(huge, 'z', sizeof(huge));
    const uint8_t normal[] = "hello";
    const uint8_t nul[] = {'a', 0, 'b', '\n'};
    const uint8_t utf8[] = {0xE3, 0x81, 0x82, 0xFF, 0};
    const size_t unchanged = (size_t)-1;
    save_case("normal", normal, sizeof(normal), 0x600, NONE, 0, 5);
    save_case("empty truncates", (const uint8_t *)"", 1, 0x600, NONE, 0, 0);
    save_case("63 bytes", max, sizeof(max), 0x600, NONE, 0, 63);
    save_case("first NUL only", nul, sizeof(nul), 0x600, NONE, 0, 1);
    save_case("UTF-8 and raw byte", utf8, sizeof(utf8), 0x600, NONE, 0, 4);
    save_case("no terminator", over, 64, 0x600, NONE, 1, unchanged);
    save_case("CR rejected", (const uint8_t *)"a\rb", 4, 0x600, NONE, 1, unchanged);
    save_case("LF rejected", (const uint8_t *)"a\nb", 4, 0x600, NONE, 1, unchanged);
    save_case("address zero", normal, sizeof(normal), 0, NONE, 0, 5);
    save_case("last valid buffer", max, sizeof(max), MEMORY_SIZE - 64, NONE, 0, 63);
    uint32_t invalid[] = {MEMORY_SIZE - 63, MEMORY_SIZE - 1, MEMORY_SIZE, UINT32_MAX};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        save_case("invalid address", normal, sizeof(normal), invalid[i], NONE, 1, unchanged);
    }
    save_case("open failure", normal, sizeof(normal), 0x600, OPEN_FAIL, 1, unchanged);
    save_case("partial write", normal, sizeof(normal), 0x600, WRITE_FAIL, 1, 4);
    save_case("close failure", normal, sizeof(normal), 0x600, CLOSE_FAIL, 1, 5);
    save_case("empty close failure", (const uint8_t *)"", 1, 0x600, CLOSE_FAIL, 1, 0);

    load_case("normal shorter replacement", normal, 5, 0x600, NONE, 0);
    load_case("empty clears", normal, 0, 0x600, NONE, 0);
    load_case("63 bytes", max, 63, 0x600, NONE, 0);
    load_case("64 rejected", over, 64, 0x600, NONE, 1);
    load_case("large rejected", huge, sizeof(huge), 0x600, NONE, 1);
    load_case("NUL rejected", nul, 3, 0x600, NONE, 1);
    load_case("CR rejected", (const uint8_t *)"a\rb", 3, 0x600, NONE, 1);
    load_case("LF rejected", (const uint8_t *)"a\nb", 3, 0x600, NONE, 1);
    load_case("UTF-8 and raw byte", utf8, 4, 0x600, NONE, 0);
    load_case("address zero", normal, 5, 0, NONE, 0);
    load_case("last valid buffer", max, 63, MEMORY_SIZE - 64, NONE, 0);
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        load_case("invalid address", normal, 5, invalid[i], NONE, 1);
    }
    load_case("open failure", normal, 5, 0x600, OPEN_FAIL, 1);
    load_case("read failure", normal, 5, 0x600, READ_FAIL, 1);
    load_case("close failure", normal, 5, 0x600, CLOSE_FAIL, 1);
    load_case("empty close failure", normal, 0, 0x600, CLOSE_FAIL, 1);
    fault = NONE;
    assert(remove("line.txt") == 0);
    memset(memory, 0xA5, sizeof(memory));
    memcpy(expected, memory, sizeof(memory));
    call(5, 0x600, 1);
    puts("load: missing file OK");
    assert(chdir("/") == 0);
    assert(rmdir(directory) == 0);
#ifdef STORAGE_VM_TEST
    printf("line-storage VM syscall: %u cases passed\n", count);
#else
    printf("line-storage component: %u cases passed\n", count);
#endif
    return 0;
}

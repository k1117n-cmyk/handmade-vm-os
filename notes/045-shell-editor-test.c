/* Component first. Build again with -DLAUNCH_VM_TEST after VM integration. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum Fault { NONE, OPEN_FAIL, READ_FAIL, PROBE_FAIL, CLOSE_FAIL, ALLOC_FAIL };
static enum Fault fault;
static unsigned opens, opened, closes;
static bool probed;
static const char *expected_path = "programs/editor.bin";
#ifdef LAUNCH_VM_TEST
static uint32_t syscall_number = 8;
#endif
#ifdef LAUNCH_VM_TEST
static unsigned allocations, frees;
static void *test_malloc(size_t size) {
    if (fault == ALLOC_FAIL) return NULL;
    void *p = malloc(size);
    if (p != NULL) allocations++;
    return p;
}
static void test_free(void *p) {
    if (p != NULL) frees++;
    free(p);
}
#endif
static FILE *test_fopen(const char *path, const char *mode) {
    assert(strcmp(path, expected_path) == 0);
    assert(strcmp(mode, "rb") == 0);
    opens++;
    FILE *f = fault == OPEN_FAIL ? NULL : fopen(path, mode);
    if (f != NULL) opened++;
    return f;
}
static size_t test_fread(void *p, size_t size, size_t count, FILE *f) {
    return fread(p, size, fault == READ_FAIL ? 1 : count, f);
}
static int test_fgetc(FILE *f) {
    probed = true;
    return fault == PROBE_FAIL ? EOF : fgetc(f);
}
static int test_ferror(FILE *f) {
    return fault == READ_FAIL || (fault == PROBE_FAIL && probed) || ferror(f);
}
static int test_fclose(FILE *f) {
    closes++;
    int result = fclose(f);
    return fault == CLOSE_FAIL ? EOF : result;
}
#define fopen test_fopen
#define fread test_fread
#define fgetc test_fgetc
#define ferror test_ferror
#define fclose test_fclose
#ifdef LAUNCH_VM_TEST
#define malloc test_malloc
#define free test_free
#define main vm_program_main
#include "vm.c"
#undef main
#undef malloc
#undef free
#else
#define MEMORY_SIZE (1024 * 1024)
#include "program-launch.h"
#endif
#undef fopen
#undef fread
#undef fgetc
#undef ferror
#undef fclose

static unsigned count;
static unsigned char data[MEMORY_SIZE + 1], memory[MEMORY_SIZE];

static void put_file(const unsigned char *bytes, size_t length) {
    FILE *f = fopen(expected_path, "wb");
    assert(f != NULL);
    assert(fwrite(bytes, 1, length, f) == length);
    assert(fclose(f) == 0);
}
static void loader_case(const char *name, size_t length, enum Fault failure,
                        bool expected) {
    put_file(data, length);
    memset(memory, 0, sizeof(memory));
    fault = failure;
    opens = opened = closes = 0;
    probed = false;
    assert(load_editor_program(memory, sizeof(memory)) == expected);
    assert(opens == 1 && closes == opened);
    if (failure == PROBE_FAIL) assert(probed);
    if (expected) {
        assert(memcmp(memory, data, length) == 0);
        for (size_t i = length; i < sizeof(memory); i++) assert(memory[i] == 0);
    }
    printf("loader: %s OK\n", name);
    count++;
}

#ifdef LAUNCH_VM_TEST
static unsigned vm_count;
static VM parent, expected_parent;
static void syscall_case(const char *name, const unsigned char *bytes, size_t length,
                         enum Fault failure, bool child, uint32_t result) {
    put_file(bytes, length);
    init_vm(&parent);
    memset(parent.memory, 0xA5, sizeof(parent.memory));
    for (unsigned i = 0; i < REGISTER_COUNT; i++) parent.regs[i] = 0xB0 + i;
    parent.sp = MEMORY_SIZE - 32;
    parent.pc = 0x200;
    parent.zero_flag = vm_count % 2 != 0;
    parent.is_child = child;
    write_inst(&parent, parent.pc, 0x60000000 | syscall_number);
    expected_parent = parent;
    expected_parent.pc += 4;
    expected_parent.regs[0] = result;
    fault = failure;
    opens = opened = closes = allocations = frees = 0;
    probed = false;
    uint32_t inst;
    assert(fetch(&parent, &inst));
    execute(&parent, decode(inst));
    assert(parent.pc == expected_parent.pc && parent.sp == expected_parent.sp);
    assert(parent.zero_flag == expected_parent.zero_flag && parent.running);
    assert(parent.is_child == child);
    assert(memcmp(parent.regs, expected_parent.regs, sizeof(parent.regs)) == 0);
    assert(memcmp(parent.memory, expected_parent.memory, MEMORY_SIZE) == 0);
    assert(opened == closes && allocations == frees);
    if (child || failure == ALLOC_FAIL) {
        assert(opens == 0 && allocations == 0);
    } else {
        assert(opens == 1 && allocations == 1);
    }
    printf("VM syscall: %s OK\n", name);
    vm_count++;
}

static void failed_load_case(const char *name, const unsigned char *bytes,
                             size_t length, enum Fault failure) {
    FILE *output = tmpfile();
    assert(output != NULL);
    assert(fflush(stdout) == 0);
    int saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout >= 0);
    assert(dup2(fileno(output), STDOUT_FILENO) >= 0);

    syscall_case(name, bytes, length, failure, false, 1);

    assert(fflush(stdout) == 0);
    assert(dup2(saved_stdout, STDOUT_FILENO) >= 0);
    assert(close(saved_stdout) == 0);
    rewind(output);
    char actual[256], expected[256];
    int size = snprintf(expected, sizeof(expected), "VM syscall: %s OK\n", name);
    assert(size >= 0 && (size_t)size < sizeof(expected));
    assert(fgets(actual, sizeof(actual), output) != NULL);
    assert(strcmp(actual, expected) == 0);
    assert(fgetc(output) == EOF && !ferror(output));
    assert(fclose(output) == 0);
    fputs(actual, stdout);
}
#endif

int main(void) {
    char directory[] = "/tmp/handmade-editor-XXXXXX";
    assert(mkdtemp(directory) != NULL && chdir(directory) == 0);
    assert(mkdir("programs", 0700) == 0);
    memset(data, 0xA5, sizeof(data));
    data[0] = 1;
    data[1] = data[2] = data[3] = 0; /* HALT at entry. */
    opens = opened = closes = 0;
    assert(!load_editor_program(memory, sizeof(memory)));
    assert(opens == 1 && opened == 0 && closes == 0);
    puts("loader: missing file OK");
    count++;
    loader_case("empty", 0, NONE, false);
    loader_case("1 byte", 1, NONE, false);
    loader_case("2 bytes", 2, NONE, false);
    loader_case("3 bytes", 3, NONE, false);
    loader_case("4 bytes", 4, NONE, true);
    loader_case("capacity minus 1", MEMORY_SIZE - 1, NONE, true);
    loader_case("capacity", MEMORY_SIZE, NONE, true);
    loader_case("capacity plus 1", MEMORY_SIZE + 1, NONE, false);
    loader_case("open failure", 4, OPEN_FAIL, false);
    loader_case("read failure", 4, READ_FAIL, false);
    loader_case("close failure", 4, CLOSE_FAIL, false);
    loader_case("probe failure", MEMORY_SIZE, PROBE_FAIL, false);
    /* The old wrapper must still select run-child.bin, not editor.bin. */
    expected_path = "programs/run-child.bin";
    put_file(data, 4);
    fault = NONE;
    opens = opened = closes = 0;
    assert(load_child_program(memory, sizeof(memory)));
    assert(opens == 1 && opened == 1 && closes == 1);
    puts("loader: old fixed run-child path OK");
    count++;
    expected_path = "programs/editor.bin";
    printf("shell-editor loader: %u cases passed\n", count);
#ifdef LAUNCH_VM_TEST
    fault = NONE;
    static VM fresh;
    init_vm(&fresh);
    assert(fresh.pc == 0 && fresh.sp == MEMORY_SIZE && !fresh.zero_flag);
    assert(fresh.running && !fresh.is_child);
    for (unsigned i = 0; i < REGISTER_COUNT; i++) assert(fresh.regs[i] == 0);
    for (size_t i = 0; i < MEMORY_SIZE; i++) assert(fresh.memory[i] == 0);
    vm_count++;
    puts("VM: initial state OK");
    const unsigned char halt[] = {1, 0, 0, 0};
    const unsigned char unknown[] = {0xFF, 0, 0, 0};
    const unsigned char nested[] = {0x60, 0, 0, 6, 1, 0, 0, 0};
    const unsigned char nested_editor[] = {
        0x60, 0, 0, 8, /* SYSCALL 8 returns 1 inside a child. */
        0x40, 0x10, 0, 1, /* MOVI R1, 1 */
        0x24, 0, 0x10, 0, /* CMP R0, R1 */
        0x6B, 0, 0, 20, /* JNZ fail */
        1, 0, 0, 0, /* HALT */
        0xFF, 0, 0, 0 /* fail */
    };
    const unsigned char eof[] = {0x60, 0, 0, 2, 1, 0, 0, 0};
    const unsigned char ret[] = {2, 0, 0, 0};
    const unsigned char clobber[] = {
        0x40, 0x10, 0x06, 0x00, /* MOVI R1, 0x600 */
        0x40, 0x00, 0x00, 0x42, /* MOVI R0, 66 */
        0x31, 0x10, 0x00, 0x00, /* STB [R1], R0 */
        0x70, 0x00, 0x00, 0x00, /* PUSH R0 */
        0x24, 0x00, 0x00, 0x00, /* CMP R0, R0 */
        1, 0, 0, 0
    };
    syscall_case("HALT", halt, sizeof(halt), NONE, false, 0);
    syscall_case("HALT full memory", data, MEMORY_SIZE, NONE, false, 0);
    syscall_case("unknown instruction", unknown, sizeof(unknown), NONE, false, 1);
    syscall_case("RET without caller", ret, sizeof(ret), NONE, false, 1);
    syscall_case("child changes memory registers stack flags", clobber, sizeof(clobber), NONE, false, 0);
    syscall_case("new child after changes", halt, sizeof(halt), NONE, false, 0);
    syscall_case("nested request returns to child", nested, sizeof(nested), NONE, false, 0);
    syscall_case("child request rejected before open", halt, sizeof(halt), NONE, true, 1);
    syscall_case("nested editor returns failure and continues", nested_editor, sizeof(nested_editor), NONE, false, 0);
    syscall_number = 6;
    syscall_case("child syscall 6 rejected before open", halt, sizeof(halt), NONE, true, 1);
    expected_path = "programs/run-child.bin";
    syscall_case("old syscall 6 still selects run-child", halt, sizeof(halt), NONE, false, 0);
    syscall_number = 8;
    expected_path = "programs/editor.bin";
    syscall_case("allocation failure", halt, sizeof(halt), ALLOC_FAIL, false, 1);
    syscall_case("open failure", halt, sizeof(halt), OPEN_FAIL, false, 1);
    failed_load_case("read failure prevents execution", halt, sizeof(halt), READ_FAIL);
    failed_load_case("close failure prevents execution", halt, sizeof(halt), CLOSE_FAIL);
    failed_load_case("probe failure prevents execution", data, MEMORY_SIZE, PROBE_FAIL);
    failed_load_case("short file prevents execution", halt, 3, NONE);
    failed_load_case("oversize prevents execution", data, sizeof(data), NONE);
    assert(freopen("/dev/null", "rb", stdin) != NULL);
    syscall_case("input EOF", eof, sizeof(eof), NONE, false, 1);
    printf("shell-editor VM: %u cases passed\n", vm_count);
#endif
    fault = NONE;
    assert(remove("programs/editor.bin") == 0);
    assert(remove("programs/run-child.bin") == 0);
    assert(rmdir("programs") == 0 && chdir("/") == 0 && rmdir(directory) == 0);
    return 0;
}

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MEMORY_SIZE (1024 * 1024)
#define REGISTER_COUNT 8

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint32_t pc;
    uint32_t sp;
    uint32_t regs[REGISTER_COUNT];
    bool zero_flag;
    bool running;
} VM;

typedef struct {
    uint32_t raw;
    uint8_t type;
    uint8_t op;
    uint8_t rd;
    uint8_t rs;
    uint32_t imm;
} DecodedInst;

static uint32_t read_u32_be(const uint8_t *memory, uint32_t address) {
    return ((uint32_t)memory[address] << 24) |
           ((uint32_t)memory[address + 1] << 16) |
           ((uint32_t)memory[address + 2] << 8) |
           ((uint32_t)memory[address + 3]);
}

static void write_u32_be(uint8_t *memory, uint32_t address, uint32_t value) {
    memory[address] = (value >> 24) & 0xFF;
    memory[address + 1] = (value >> 16) & 0xFF;
    memory[address + 2] = (value >> 8) & 0xFF;
    memory[address + 3] = value & 0xFF;
}

static void write_inst(VM *vm, uint32_t address, uint32_t inst) {
    write_u32_be(vm->memory, address, inst);
}

static bool load_program_file(VM *vm, const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        perror(path);
        return false;
    }

    size_t bytes_read = fread(vm->memory, 1, MEMORY_SIZE, file);

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

static DecodedInst decode(uint32_t inst) {
    DecodedInst decoded;

    decoded.raw = inst;
    decoded.type = (inst >> 28) & 0x0F;
    decoded.op = (inst >> 24) & 0x0F;
    decoded.rd = (inst >> 20) & 0x0F;
    decoded.rs = (inst >> 16) & 0x0F;
    decoded.imm = inst & 0x000FFFFF;

    return decoded;
}

static bool fetch(VM *vm, uint32_t *inst) {
    if (vm->pc > MEMORY_SIZE - 4) {
        printf("pc out of range: 0x%08X\n", vm->pc);
        vm->running = false;
        return false;
    }

    *inst = read_u32_be(vm->memory, vm->pc);
    vm->pc += 4;
    return true;
}

static bool check_register(uint8_t reg) {
    if (reg >= REGISTER_COUNT) {
        printf("invalid register: R%u\n", reg);
        return false;
    }

    return true;
}

static bool check_register_pair(uint8_t rd, uint8_t rs) {
    if (rd >= REGISTER_COUNT || rs >= REGISTER_COUNT) {
        printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
        return false;
    }

    return true;
}

static bool check_u32_memory_range(uint32_t address) {
    if (address > MEMORY_SIZE - 4) {
        printf("memory address out of range: 0x%08X\n", address);
        return false;
    }

    return true;
}

static bool check_jump_target(uint32_t address) {
    if (address > MEMORY_SIZE - 4) {
        printf("jump target out of range: 0x%08X\n", address);
        return false;
    }

    return true;
}

static void print_string(VM *vm, uint32_t address) {
    while (address < MEMORY_SIZE && vm->memory[address] != 0) {
        putchar(vm->memory[address]);
        address++;
    }
}

static void execute(VM *vm, DecodedInst inst) {
    if (inst.raw == 0x01000000) {
        vm->running = false;
        printf("CPU halted.\n");
    } else if (inst.raw == 0x02000000) {
        if (vm->sp > MEMORY_SIZE - 4) {
            printf("stack underflow\n");
            vm->running = false;
        } else {
            uint32_t return_address = read_u32_be(vm->memory, vm->sp);
            vm->sp += 4;

            if (!check_jump_target(return_address)) {
                vm->running = false;
            } else {
                vm->pc = return_address;
            }
        }
    } else if (inst.type == 1 && inst.op == 0) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] = vm->regs[inst.rs];
        }
    } else if (inst.type == 2 && inst.op == 0) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] += 1;
        }
    } else if (inst.type == 2 && inst.op == 1) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] -= 1;
        }
    } else if (inst.type == 2 && inst.op == 2) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] += vm->regs[inst.rs];
        }
    } else if (inst.type == 2 && inst.op == 3) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] -= vm->regs[inst.rs];
        }
    } else if (inst.type == 2 && inst.op == 4) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else {
            vm->zero_flag = vm->regs[inst.rd] == vm->regs[inst.rs];
        }
    } else if (inst.type == 3 && inst.op == 0) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else if (vm->regs[inst.rs] >= MEMORY_SIZE) {
            printf("memory address out of range: 0x%08X\n", vm->regs[inst.rs]);
            vm->running = false;
        } else {
            vm->regs[inst.rd] = vm->memory[vm->regs[inst.rs]];
        }
    } else if (inst.type == 3 && inst.op == 1) {
        if (!check_register_pair(inst.rd, inst.rs)) {
            vm->running = false;
        } else if (vm->regs[inst.rd] >= MEMORY_SIZE) {
            printf("memory address out of range: 0x%08X\n", vm->regs[inst.rd]);
            vm->running = false;
        } else {
            vm->memory[vm->regs[inst.rd]] = vm->regs[inst.rs] & 0xFF;
        }
    } else if (inst.type == 4 && inst.op == 0) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] = inst.imm;
        }
    } else if (inst.type == 5 && inst.op == 0) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else if (!check_u32_memory_range(inst.imm)) {
            vm->running = false;
        } else {
            vm->regs[inst.rd] = read_u32_be(vm->memory, inst.imm);
        }
    } else if (inst.type == 5 && inst.op == 1) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else if (!check_u32_memory_range(inst.imm)) {
            vm->running = false;
        } else {
            write_u32_be(vm->memory, inst.imm, vm->regs[inst.rd]);
        }
    } else if (inst.type == 6 && inst.op == 0) {
        if (inst.imm == 0) {
            putchar(vm->regs[0] & 0xFF);
            putchar('\n');
        } else if (inst.imm == 1) {
            print_string(vm, vm->regs[0]);
        } else {
            printf("unimplemented syscall: %u\n", inst.imm);
            vm->running = false;
        }
    } else if (inst.type == 6 && inst.op == 8) {
        if (!check_jump_target(inst.imm)) {
            vm->running = false;
        } else {
            vm->pc = inst.imm;
        }
    } else if (inst.type == 6 && inst.op == 9) {
        if (!check_jump_target(inst.imm)) {
            vm->running = false;
        } else if (vm->sp < 4) {
            printf("stack overflow\n");
            vm->running = false;
        } else {
            vm->sp -= 4;
            write_u32_be(vm->memory, vm->sp, vm->pc);
            vm->pc = inst.imm;
        }
    } else if (inst.type == 6 && inst.op == 10) {
        if (vm->zero_flag) {
            if (!check_jump_target(inst.imm)) {
                vm->running = false;
            } else {
                vm->pc = inst.imm;
            }
        }
    } else if (inst.type == 6 && inst.op == 11) {
        if (!vm->zero_flag) {
            if (!check_jump_target(inst.imm)) {
                vm->running = false;
            } else {
                vm->pc = inst.imm;
            }
        }
    } else if (inst.type == 7 && inst.op == 0) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else if (vm->sp < 4) {
            printf("stack overflow\n");
            vm->running = false;
        } else {
            vm->sp -= 4;
            write_u32_be(vm->memory, vm->sp, vm->regs[inst.rd]);
        }
    } else if (inst.type == 7 && inst.op == 1) {
        if (!check_register(inst.rd)) {
            vm->running = false;
        } else if (vm->sp > MEMORY_SIZE - 4) {
            printf("stack underflow\n");
            vm->running = false;
        } else {
            vm->regs[inst.rd] = read_u32_be(vm->memory, vm->sp);
            vm->sp += 4;
        }
    } else {
        printf("unknown instruction: %08X\n", inst.raw);
        vm->running = false;
    }
}

static void run(VM *vm) {
    while (vm->running) {
        uint32_t inst;

        if (!fetch(vm, &inst)) {
            return;
        }

        execute(vm, decode(inst));
    }
}

static void load_test_program(VM *vm) {
    write_inst(vm, 0x00000000, 0x40100080);  // MOVI R1, 0x80
    write_inst(vm, 0x00000004, 0x40000041);  // MOVI R0, 65
    write_inst(vm, 0x00000008, 0x31100000);  // STB [R1], R0
    write_inst(vm, 0x0000000C, 0x30010000);  // LDB R0, [R1]
    write_inst(vm, 0x00000010, 0x60000000);  // SYSCALL 0
    write_inst(vm, 0x00000014, 0x50200090);  // LDDI R2, 0x90
    write_inst(vm, 0x00000018, 0x51200040);  // STDI R2, 0x40
    write_inst(vm, 0x0000001C, 0x20200000);  // INC R2
    write_inst(vm, 0x00000020, 0x21200000);  // DEC R2
    write_inst(vm, 0x00000024, 0x10320000);  // MOV R3, R2
    write_inst(vm, 0x00000028, 0x22230000);  // ADD R2, R3
    write_inst(vm, 0x0000002C, 0x23230000);  // SUB R2, R3
    write_inst(vm, 0x00000030, 0x24230000);  // CMP R2, R3
    write_inst(vm, 0x00000034, 0x6A000050);  // JZ 0x50
    write_inst(vm, 0x00000038, 0x68000054);  // JUMP 0x54
    write_inst(vm, 0x00000050, 0x4000005A);  // MOVI R0, 90
    write_inst(vm, 0x00000054, 0x40500001);  // MOVI R5, 1
    write_inst(vm, 0x00000058, 0x24050000);  // CMP R0, R5
    write_inst(vm, 0x0000005C, 0x6B000068);  // JNZ 0x68
    write_inst(vm, 0x00000060, 0x6800006C);  // JUMP 0x6C
    write_inst(vm, 0x00000068, 0x70300000);  // PUSH R3
    write_inst(vm, 0x0000006C, 0x71400000);  // POP R4
    write_inst(vm, 0x00000070, 0x690000A0);  // CALLI 0xA0
    write_inst(vm, 0x00000074, 0x40000100);  // MOVI R0, 0x100
    write_inst(vm, 0x00000078, 0x60000001);  // SYSCALL 1
    write_inst(vm, 0x0000007C, 0x01000000);  // HALT
    write_inst(vm, 0x000000A0, 0x4060002A);  // MOVI R6, 42
    write_inst(vm, 0x000000A4, 0x02000000);  // RET

    write_u32_be(vm->memory, 0x00000090, 0x12345678);

    vm->memory[0x00000100] = 'V';
    vm->memory[0x00000101] = 'M';
    vm->memory[0x00000102] = ' ';
    vm->memory[0x00000103] = 'f';
    vm->memory[0x00000104] = 'l';
    vm->memory[0x00000105] = 'o';
    vm->memory[0x00000106] = 'w';
    vm->memory[0x00000107] = ' ';
    vm->memory[0x00000108] = 'c';
    vm->memory[0x00000109] = 'o';
    vm->memory[0x0000010A] = 'm';
    vm->memory[0x0000010B] = 'p';
    vm->memory[0x0000010C] = 'l';
    vm->memory[0x0000010D] = 'e';
    vm->memory[0x0000010E] = 't';
    vm->memory[0x0000010F] = 'e';
    vm->memory[0x00000110] = '.';
    vm->memory[0x00000111] = '\n';
    vm->memory[0x00000112] = 0x00;
}

int main(int argc, char **argv) {
    VM vm = {0};

    if (argc > 2) {
        fprintf(stderr, "usage: %s [program.bin]\n", argv[0]);
        return 1;
    }

    vm.pc = 0x00000000;
    vm.sp = 0x00100000;
    vm.running = true;

    if (argc == 2) {
        if (!load_program_file(&vm, argv[1])) {
            return 1;
        }
    } else {
        load_test_program(&vm);
    }

    run(&vm);

    return 0;
}

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

static void execute(VM *vm, DecodedInst inst) {
    if (inst.raw == 0x01000000) {
        vm->running = false;
        printf("CPU halted.\n");
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
        } else {
            printf("unimplemented syscall: %u\n", inst.imm);
            vm->running = false;
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
    write_inst(vm, 0x00000028, 0x70300000);  // PUSH R3
    write_inst(vm, 0x0000002C, 0x71400000);  // POP R4
    write_inst(vm, 0x00000030, 0x01000000);  // HALT

    write_u32_be(vm->memory, 0x00000090, 0x12345678);
}

int main(void) {
    VM vm = {0};

    vm.pc = 0x00000000;
    vm.sp = 0x00100000;
    vm.running = true;

    load_test_program(&vm);
    run(&vm);

    return 0;
}

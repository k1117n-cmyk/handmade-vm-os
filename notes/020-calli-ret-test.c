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

static void write_u32_be(uint8_t *memory, uint32_t address, uint32_t value) {
    memory[address] = (value >> 24) & 0xFF;
    memory[address + 1] = (value >> 16) & 0xFF;
    memory[address + 2] = (value >> 8) & 0xFF;
    memory[address + 3] = value & 0xFF;
}

static bool can_fetch(uint32_t address) {
    return address <= MEMORY_SIZE - 4;
}

int main(void) {
    uint8_t memory[MEMORY_SIZE] = {0};
    uint32_t pc = 0x00000000;
    uint32_t sp = 0x00100000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x69;  // CALLI 0x20
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x20;

    memory[0x00000004] = 0x40;  // MOVI R1, 7
    memory[0x00000005] = 0x10;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x07;

    memory[0x00000008] = 0x01;  // HALT
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x00000020] = 0x40;  // MOVI R0, 42
    memory[0x00000021] = 0x00;
    memory[0x00000022] = 0x00;
    memory[0x00000023] = 0x2A;

    memory[0x00000024] = 0x02;  // RET
    memory[0x00000025] = 0x00;
    memory[0x00000026] = 0x00;
    memory[0x00000027] = 0x00;

    while (running) {
        if (!can_fetch(pc)) {
            printf("pc out of range: 0x%08X\n", pc);
            running = false;
            break;
        }

        uint32_t inst = read_u32_be(memory, pc);

        pc += 4;

        uint8_t type = (inst >> 28) & 0x0F;
        uint8_t op = (inst >> 24) & 0x0F;
        uint8_t rd = (inst >> 20) & 0x0F;
        uint32_t imm = inst & 0x000FFFFF;

        if (inst == 0x01000000) {
            running = false;
        } else if (inst == 0x02000000) {
            if (sp > MEMORY_SIZE - 4) {
                printf("stack underflow\n");
                running = false;
            } else {
                uint32_t return_address = read_u32_be(memory, sp);
                sp += 4;

                if (!can_fetch(return_address)) {
                    printf("return address out of range: 0x%08X\n", return_address);
                    running = false;
                } else {
                    pc = return_address;
                }
            }
        } else if (type == 4 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] = imm;
            }
        } else if (type == 6 && op == 9) {
            if (!can_fetch(imm)) {
                printf("call target out of range: 0x%08X\n", imm);
                running = false;
            } else if (sp < 4) {
                printf("stack overflow\n");
                running = false;
            } else {
                sp -= 4;
                write_u32_be(memory, sp, pc);
                pc = imm;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);
    printf("R1=0x%08X\n", regs[1]);
    printf("SP=0x%08X\n", sp);

    if (regs[0] == 42 && regs[1] == 7 && sp == 0x00100000) {
        printf("CALLI/RET test passed.\n");
        return 0;
    }

    printf("CALLI/RET test failed.\n");
    return 1;
}

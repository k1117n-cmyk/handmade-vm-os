#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MEMORY_SIZE (1024 * 1024)

static bool can_fetch(uint32_t address) {
    return address <= MEMORY_SIZE - 4;
}

int main(void) {
    uint8_t memory[MEMORY_SIZE] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool zero_flag = false;
    bool running = true;

    memory[0x00000000] = 0x68;  // JUMP 0x10
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x10;

    memory[0x00000004] = 0x40;  // MOVI R0, 99
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x63;

    memory[0x00000008] = 0x01;  // HALT
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x00000010] = 0x40;  // MOVI R0, 1
    memory[0x00000011] = 0x00;
    memory[0x00000012] = 0x00;
    memory[0x00000013] = 0x01;

    memory[0x00000014] = 0x40;  // MOVI R1, 1
    memory[0x00000015] = 0x10;
    memory[0x00000016] = 0x00;
    memory[0x00000017] = 0x01;

    memory[0x00000018] = 0x24;  // CMP R0, R1
    memory[0x00000019] = 0x01;
    memory[0x0000001A] = 0x00;
    memory[0x0000001B] = 0x00;

    memory[0x0000001C] = 0x6A;  // JZ 0x28
    memory[0x0000001D] = 0x00;
    memory[0x0000001E] = 0x00;
    memory[0x0000001F] = 0x28;

    memory[0x00000020] = 0x40;  // MOVI R2, 99
    memory[0x00000021] = 0x20;
    memory[0x00000022] = 0x00;
    memory[0x00000023] = 0x63;

    memory[0x00000024] = 0x01;  // HALT
    memory[0x00000025] = 0x00;
    memory[0x00000026] = 0x00;
    memory[0x00000027] = 0x00;

    memory[0x00000028] = 0x40;  // MOVI R2, 7
    memory[0x00000029] = 0x20;
    memory[0x0000002A] = 0x00;
    memory[0x0000002B] = 0x07;

    memory[0x0000002C] = 0x01;  // HALT
    memory[0x0000002D] = 0x00;
    memory[0x0000002E] = 0x00;
    memory[0x0000002F] = 0x00;

    while (running) {
        if (!can_fetch(pc)) {
            printf("pc out of range: 0x%08X\n", pc);
            running = false;
            break;
        }

        uint32_t inst =
            ((uint32_t)memory[pc] << 24) |
            ((uint32_t)memory[pc + 1] << 16) |
            ((uint32_t)memory[pc + 2] << 8) |
            ((uint32_t)memory[pc + 3]);

        pc += 4;

        uint8_t type = (inst >> 28) & 0x0F;
        uint8_t op = (inst >> 24) & 0x0F;
        uint8_t rd = (inst >> 20) & 0x0F;
        uint8_t rs = (inst >> 16) & 0x0F;
        uint32_t imm = inst & 0x000FFFFF;

        if (inst == 0x01000000) {
            running = false;
        } else if (type == 2 && op == 4) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else {
                zero_flag = regs[rd] == regs[rs];
            }
        } else if (type == 4 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] = imm;
            }
        } else if (type == 6 && op == 8) {
            if (!can_fetch(imm)) {
                printf("jump target out of range: 0x%08X\n", imm);
                running = false;
            } else {
                pc = imm;
            }
        } else if (type == 6 && op == 10) {
            if (zero_flag) {
                if (!can_fetch(imm)) {
                    printf("jump target out of range: 0x%08X\n", imm);
                    running = false;
                } else {
                    pc = imm;
                }
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);
    printf("R1=0x%08X\n", regs[1]);
    printf("R2=0x%08X\n", regs[2]);
    printf("zero_flag=%s\n", zero_flag ? "true" : "false");

    if (regs[0] == 1 && regs[1] == 1 && regs[2] == 7 && zero_flag) {
        printf("JUMP/JZ test passed.\n");
        return 0;
    }

    printf("JUMP/JZ test failed.\n");
    return 1;
}

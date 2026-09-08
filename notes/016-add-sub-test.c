#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R0, 10
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x0A;

    memory[0x00000004] = 0x40;  // MOVI R1, 3
    memory[0x00000005] = 0x10;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x03;

    memory[0x00000008] = 0x22;  // ADD R0, R1
    memory[0x00000009] = 0x01;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x0000000C] = 0x40;  // MOVI R2, 10
    memory[0x0000000D] = 0x20;
    memory[0x0000000E] = 0x00;
    memory[0x0000000F] = 0x0A;

    memory[0x00000010] = 0x40;  // MOVI R3, 3
    memory[0x00000011] = 0x30;
    memory[0x00000012] = 0x00;
    memory[0x00000013] = 0x03;

    memory[0x00000014] = 0x23;  // SUB R2, R3
    memory[0x00000015] = 0x23;
    memory[0x00000016] = 0x00;
    memory[0x00000017] = 0x00;

    memory[0x00000018] = 0x01;  // HALT
    memory[0x00000019] = 0x00;
    memory[0x0000001A] = 0x00;
    memory[0x0000001B] = 0x00;

    while (running) {
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
        } else if (type == 2 && op == 2) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else {
                regs[rd] += regs[rs];
            }
        } else if (type == 2 && op == 3) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else {
                regs[rd] -= regs[rs];
            }
        } else if (type == 4 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] = imm;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);
    printf("R2=0x%08X\n", regs[2]);

    if (regs[0] == 0x0000000D && regs[2] == 0x00000007) {
        printf("ADD/SUB test passed.\n");
        return 0;
    }

    printf("ADD/SUB test failed.\n");
    return 1;
}

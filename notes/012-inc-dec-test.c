#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R0, 0x10
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x10;

    memory[0x00000004] = 0x40;  // MOVI R1, 0x10
    memory[0x00000005] = 0x10;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x10;

    memory[0x00000008] = 0x20;  // INC R0
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x0000000C] = 0x21;  // DEC R1
    memory[0x0000000D] = 0x10;
    memory[0x0000000E] = 0x00;
    memory[0x0000000F] = 0x00;

    memory[0x00000010] = 0x01;  // HALT
    memory[0x00000011] = 0x00;
    memory[0x00000012] = 0x00;
    memory[0x00000013] = 0x00;

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
        uint32_t imm = inst & 0x000FFFFF;

        if (inst == 0x01000000) {
            running = false;
        } else if (type == 2 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] += 1;
            }
        } else if (type == 2 && op == 1) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] -= 1;
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
    printf("R1=0x%08X\n", regs[1]);

    if (regs[0] == 0x00000011 && regs[1] == 0x0000000F) {
        printf("INC/DEC test passed.\n");
        return 0;
    }

    printf("INC/DEC test failed.\n");
    return 1;
}

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x50;  // LDDI R0, 0x10
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x10;

    memory[0x00000004] = 0x01;  // HALT
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x00;

    memory[0x00000010] = 0x12;
    memory[0x00000011] = 0x34;
    memory[0x00000012] = 0x56;
    memory[0x00000013] = 0x78;

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
        } else if (type == 5 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else if (imm > sizeof(memory) - 4) {
                printf("memory address out of range: 0x%08X\n", imm);
                running = false;
            } else {
                regs[rd] =
                    ((uint32_t)memory[imm] << 24) |
                    ((uint32_t)memory[imm + 1] << 16) |
                    ((uint32_t)memory[imm + 2] << 8) |
                    ((uint32_t)memory[imm + 3]);
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);

    if (regs[0] == 0x12345678) {
        printf("LDDI test passed.\n");
        return 0;
    }

    printf("LDDI test failed.\n");
    return 1;
}

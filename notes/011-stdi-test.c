#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    regs[0] = 0x12345678;

    memory[0x00000000] = 0x51;  // STDI R0, 0x10
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x10;

    memory[0x00000004] = 0x01;  // HALT
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x00;

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
        } else if (type == 5 && op == 1) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else if (imm > sizeof(memory) - 4) {
                printf("memory address out of range: 0x%08X\n", imm);
                running = false;
            } else {
                memory[imm] = (regs[rd] >> 24) & 0xFF;
                memory[imm + 1] = (regs[rd] >> 16) & 0xFF;
                memory[imm + 2] = (regs[rd] >> 8) & 0xFF;
                memory[imm + 3] = regs[rd] & 0xFF;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);
    printf("memory[0x10..0x13]=%02X %02X %02X %02X\n",
           memory[0x10],
           memory[0x11],
           memory[0x12],
           memory[0x13]);

    if (memory[0x10] == 0x12 &&
        memory[0x11] == 0x34 &&
        memory[0x12] == 0x56 &&
        memory[0x13] == 0x78) {
        printf("STDI test passed.\n");
        return 0;
    }

    printf("STDI test failed.\n");
    return 1;
}

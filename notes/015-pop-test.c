#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t sp = 0x000FFFFC;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x71;  // POP R1
    memory[0x00000001] = 0x10;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x00;

    memory[0x00000004] = 0x01;  // HALT
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x00;

    memory[0x000FFFFC] = 0x12;
    memory[0x000FFFFD] = 0x34;
    memory[0x000FFFFE] = 0x56;
    memory[0x000FFFFF] = 0x78;

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

        if (inst == 0x01000000) {
            running = false;
        } else if (type == 7 && op == 1) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else if (sp > sizeof(memory) - 4) {
                printf("stack underflow\n");
                running = false;
            } else {
                regs[rd] =
                    ((uint32_t)memory[sp] << 24) |
                    ((uint32_t)memory[sp + 1] << 16) |
                    ((uint32_t)memory[sp + 2] << 8) |
                    ((uint32_t)memory[sp + 3]);
                sp += 4;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R1=0x%08X\n", regs[1]);
    printf("SP=0x%08X\n", sp);

    if (regs[1] == 0x12345678 && sp == 0x00100000) {
        printf("POP test passed.\n");
        return 0;
    }

    printf("POP test failed.\n");
    return 1;
}

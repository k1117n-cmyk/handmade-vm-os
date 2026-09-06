#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t sp = 0x00100000;
    uint32_t regs[8] = {0};
    bool running = true;

    regs[0] = 0x12345678;

    memory[0x00000000] = 0x70;  // PUSH R0
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x00;

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

        if (inst == 0x01000000) {
            running = false;
        } else if (type == 7 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else if (sp < 4) {
                printf("stack overflow\n");
                running = false;
            } else {
                sp -= 4;
                memory[sp] = (regs[rd] >> 24) & 0xFF;
                memory[sp + 1] = (regs[rd] >> 16) & 0xFF;
                memory[sp + 2] = (regs[rd] >> 8) & 0xFF;
                memory[sp + 3] = regs[rd] & 0xFF;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    printf("R0=0x%08X\n", regs[0]);
    printf("SP=0x%08X\n", sp);
    printf("memory[SP..SP+3]=%02X %02X %02X %02X\n",
           memory[sp],
           memory[sp + 1],
           memory[sp + 2],
           memory[sp + 3]);

    if (sp == 0x000FFFFC &&
        memory[sp] == 0x12 &&
        memory[sp + 1] == 0x34 &&
        memory[sp + 2] == 0x56 &&
        memory[sp + 3] == 0x78) {
        printf("PUSH test passed.\n");
        return 0;
    }

    printf("PUSH test failed.\n");
    return 1;
}

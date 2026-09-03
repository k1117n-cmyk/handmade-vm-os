#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R0, 65
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x41;

    memory[0x00000004] = 0x60;  // SYSCALL 0
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x00;

    memory[0x00000008] = 0x01;  // HALT
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

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
            printf("CPU halted.\n");
        } else if (type == 4 && op == 0) {
            regs[rd] = imm;
        } else if (type == 6 && op == 0) {
            if (imm == 0) {
                putchar(regs[0] & 0xFF);
                putchar('\n');
            } else {
                printf("unimplemented syscall: %u\n", imm);
                running = false;
            }
        }
    }

    return 0;
}

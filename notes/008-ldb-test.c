#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R1, 0x10
    memory[0x00000001] = 0x10;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x10;

    memory[0x00000004] = 0x30;  // LDB R0, [R1]
    memory[0x00000005] = 0x01;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x00;

    memory[0x00000008] = 0x60;  // SYSCALL 0
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x0000000C] = 0x01;  // HALT
    memory[0x0000000D] = 0x00;
    memory[0x0000000E] = 0x00;
    memory[0x0000000F] = 0x00;

    memory[0x00000010] = 0x41;  // 'A'

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
            printf("CPU halted.\n");
        } else if (type == 3 && op == 0) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else if (regs[rs] >= sizeof(memory)) {
                printf("memory address out of range: 0x%08X\n", regs[rs]);
                running = false;
            } else {
                regs[rd] = memory[regs[rs]];
            }
        } else if (type == 4 && op == 0) {
            if (rd >= 8) {
                printf("invalid register: R%u\n", rd);
                running = false;
            } else {
                regs[rd] = imm;
            }
        } else if (type == 6 && op == 0) {
            if (imm == 0) {
                putchar(regs[0] & 0xFF);
                putchar('\n');
            } else {
                printf("unimplemented syscall: %u\n", imm);
                running = false;
            }
        } else {
            printf("unknown instruction: %08X\n", inst);
            running = false;
        }
    }

    return 0;
}

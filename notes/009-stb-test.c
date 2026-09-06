#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R1, 0x20
    memory[0x00000001] = 0x10;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x20;

    memory[0x00000004] = 0x40;  // MOVI R0, 65
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x41;

    memory[0x00000008] = 0x31;  // STB [R1], R0
    memory[0x00000009] = 0x10;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x0000000C] = 0x30;  // LDB R2, [R1]
    memory[0x0000000D] = 0x21;
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
        uint8_t rs = (inst >> 16) & 0x0F;
        uint32_t imm = inst & 0x000FFFFF;

        if (inst == 0x01000000) {
            running = false;
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
        } else if (type == 3 && op == 1) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else if (regs[rd] >= sizeof(memory)) {
                printf("memory address out of range: 0x%08X\n", regs[rd]);
                running = false;
            } else {
                memory[regs[rd]] = regs[rs] & 0xFF;
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

    printf("R1=0x%08X\n", regs[1]);
    printf("R0=0x%08X\n", regs[0]);
    printf("memory[0x20]=0x%02X\n", memory[0x20]);
    printf("R2=0x%08X\n", regs[2]);

    if (memory[0x20] == 0x41 && regs[2] == 0x41) {
        printf("STB test passed.\n");
        return 0;
    }

    printf("STB test failed.\n");
    return 1;
}

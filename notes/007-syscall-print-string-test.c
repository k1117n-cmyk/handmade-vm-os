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

    memory[0x00000004] = 0x60;  // SYSCALL 1
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x01;

    memory[0x00000008] = 0x01;  // HALT
    memory[0x00000009] = 0x00;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x00000010] = 'H';
    memory[0x00000011] = 'e';
    memory[0x00000012] = 'l';
    memory[0x00000013] = 'l';
    memory[0x00000014] = 'o';
    memory[0x00000015] = ' ';
    memory[0x00000016] = 'f';
    memory[0x00000017] = 'r';
    memory[0x00000018] = 'o';
    memory[0x00000019] = 'm';
    memory[0x0000001A] = ' ';
    memory[0x0000001B] = 'H';
    memory[0x0000001C] = 'a';
    memory[0x0000001D] = 'n';
    memory[0x0000001E] = 'd';
    memory[0x0000001F] = 'm';
    memory[0x00000020] = 'a';
    memory[0x00000021] = 'd';
    memory[0x00000022] = 'e';
    memory[0x00000023] = ' ';
    memory[0x00000024] = 'O';
    memory[0x00000025] = 'S';
    memory[0x00000026] = '\n';
    memory[0x00000027] = 0x00;

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
            } else if (imm == 1) {
                uint32_t address = regs[0];
                while (address < sizeof(memory) && memory[address] != 0) {
                    putchar(memory[address]);
                    address++;
                }
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

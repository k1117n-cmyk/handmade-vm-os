#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t sp = 0x00100000;
    uint32_t regs[8] = {0};
    bool running = true;

    memory[0x00000000] = 0x40;  // MOVI R1, 0x80
    memory[0x00000001] = 0x10;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x80;

    memory[0x00000004] = 0x40;  // MOVI R0, 65
    memory[0x00000005] = 0x00;
    memory[0x00000006] = 0x00;
    memory[0x00000007] = 0x41;

    memory[0x00000008] = 0x31;  // STB [R1], R0
    memory[0x00000009] = 0x10;
    memory[0x0000000A] = 0x00;
    memory[0x0000000B] = 0x00;

    memory[0x0000000C] = 0x30;  // LDB R0, [R1]
    memory[0x0000000D] = 0x01;
    memory[0x0000000E] = 0x00;
    memory[0x0000000F] = 0x00;

    memory[0x00000010] = 0x60;  // SYSCALL 0
    memory[0x00000011] = 0x00;
    memory[0x00000012] = 0x00;
    memory[0x00000013] = 0x00;

    memory[0x00000014] = 0x50;  // LDDI R2, 0x90
    memory[0x00000015] = 0x20;
    memory[0x00000016] = 0x00;
    memory[0x00000017] = 0x90;

    memory[0x00000018] = 0x51;  // STDI R2, 0x40
    memory[0x00000019] = 0x20;
    memory[0x0000001A] = 0x00;
    memory[0x0000001B] = 0x40;

    memory[0x0000001C] = 0x20;  // INC R2
    memory[0x0000001D] = 0x20;
    memory[0x0000001E] = 0x00;
    memory[0x0000001F] = 0x00;

    memory[0x00000020] = 0x21;  // DEC R2
    memory[0x00000021] = 0x20;
    memory[0x00000022] = 0x00;
    memory[0x00000023] = 0x00;

    memory[0x00000024] = 0x10;  // MOV R3, R2
    memory[0x00000025] = 0x32;
    memory[0x00000026] = 0x00;
    memory[0x00000027] = 0x00;

    memory[0x00000028] = 0x70;  // PUSH R3
    memory[0x00000029] = 0x30;
    memory[0x0000002A] = 0x00;
    memory[0x0000002B] = 0x00;

    memory[0x0000002C] = 0x71;  // POP R4
    memory[0x0000002D] = 0x40;
    memory[0x0000002E] = 0x00;
    memory[0x0000002F] = 0x00;

    memory[0x00000030] = 0x01;  // HALT
    memory[0x00000031] = 0x00;
    memory[0x00000032] = 0x00;
    memory[0x00000033] = 0x00;

    memory[0x00000090] = 0x12;
    memory[0x00000091] = 0x34;
    memory[0x00000092] = 0x56;
    memory[0x00000093] = 0x78;

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
        } else if (type == 1 && op == 0) {
            if (rd >= 8 || rs >= 8) {
                printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
                running = false;
            } else {
                regs[rd] = regs[rs];
            }
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
        } else if (type == 6 && op == 0) {
            if (imm == 0) {
                putchar(regs[0] & 0xFF);
                putchar('\n');
            } else {
                printf("unimplemented syscall: %u\n", imm);
                running = false;
            }
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

    return 0;
}

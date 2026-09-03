#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t regs[8] = {0};

    memory[0x00000000] = 0x40;
    memory[0x00000001] = 0x71;
    memory[0x00000002] = 0x23;
    memory[0x00000003] = 0x45;


    uint32_t inst =
        ((uint32_t)memory[pc] << 24) |
        ((uint32_t)memory[pc + 1] << 16) |
        ((uint32_t)memory[pc + 2] << 8) |
        ((uint32_t)memory[pc + 3]);

    pc += 4;
    uint8_t opcode = (inst >> 24) & 0xFF;
    uint8_t rd = (inst >> 20) & 0x0F;
    uint32_t imm = inst & 0x000FFFFF;

    if (opcode == 0x40) {
	    regs[rd] = imm;
    }
    printf("inst=%08X\n", inst);
    printf("R7=%05X\n", regs[7]);

    return 0;
}

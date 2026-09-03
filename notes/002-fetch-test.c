#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;

    memory[0x00000000] = 0x01;
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x00;

    printf("PC before=%08X\n", pc);

    uint32_t inst =
        ((uint32_t)memory[pc] << 24) |
        ((uint32_t)memory[pc + 1] << 16) |
        ((uint32_t)memory[pc + 2] << 8) |
        ((uint32_t)memory[pc + 3]);

    pc += 4;

    uint8_t opcode = memory[0x00000000];

    printf("INST=%08X\n", inst);
    printf("OPCODE=%02X\n", opcode);
    printf("PC after =%08X\n", pc);

    return 0;
}

#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint32_t instruction = 0x40780010;

    uint8_t b0 = (instruction >> 24) & 0xFF;
    uint8_t b1 = (instruction >> 16) & 0xFF;

    uint8_t type = (b0 >> 4) & 0x0F;
    uint8_t op   = b0 & 0x0F;
    uint8_t rd   = (b1 >> 4) & 0x0F;

    uint32_t imm = instruction & 0x000FFFFF;

    printf("instruction=%08X\n", instruction);
    printf("b0=%02X\n", b0);
    printf("b1=%02X\n", b1);
    printf("type=%u\n", type);
    printf("op=%u\n", op);
    printf("rd=%u\n", rd);
    printf("imm=%05X\n", imm);

    return 0;
}

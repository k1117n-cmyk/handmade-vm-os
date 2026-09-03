#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    uint32_t sp = 0x00100000;

    memory[0x00000000] = 0x01;
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x00;

    printf("PC=%08X\n", pc);
    printf("SP=%08X\n", sp);

    return 0;
}

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    uint8_t memory[1024 * 1024] = {0};
    uint32_t pc = 0x00000000;
    bool running = true;

    memory[0x00000000] = 0x01;
    memory[0x00000001] = 0x00;
    memory[0x00000002] = 0x00;
    memory[0x00000003] = 0x00;

    while (running) {
        uint32_t inst =
            ((uint32_t)memory[pc] << 24) |
            ((uint32_t)memory[pc + 1] << 16) |
            ((uint32_t)memory[pc + 2] << 8) |
            ((uint32_t)memory[pc + 3]);

        pc += 4;

        uint8_t opcode = (inst >> 24) & 0xff;

        if (opcode == 0x01) {
            running = false;
            printf("CPU halted.\n");
        }
    }

    return 0;
}

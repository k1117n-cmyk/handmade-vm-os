; Day 38: the same bytes as asm-memory-call.asm, with forward references.
MOVI R1, 0x500
MOVI R0, 65
STB [R1], R0
INC R1
MOVI R0, 66
STB [R1], R0
MOVI R1, 0x500
CALLI print_byte
CALLI print_byte
HALT

print_byte:
LDB R0, [R1]
SYSCALL 0
INC R1
RET

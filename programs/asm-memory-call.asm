; Day 37: store A/B, then call the same byte-printing routine twice.
MOVI R1, 0x500       ; 0x00
MOVI R0, 65          ; 0x04
STB [R1], R0        ; 0x08
INC R1              ; 0x0C
MOVI R0, 66          ; 0x10
STB [R1], R0        ; 0x14
MOVI R1, 0x500       ; 0x18
CALLI 0x28          ; 0x1C
CALLI 0x28          ; 0x20
HALT                ; 0x24
LDB R0, [R1]        ; 0x28
SYSCALL 0           ; 0x2C
INC R1              ; 0x30
RET                 ; 0x34

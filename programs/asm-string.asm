; Day 39: jump over data, then print a zero-terminated string.
JUMP start
message: .string "Hello, VM!\n"
.align 4
start:
MOVI R0, message
SYSCALL 1
HALT

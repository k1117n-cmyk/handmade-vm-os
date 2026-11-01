; Day 40: code first, then packed strings. Buffer: 0x500..0x53F.
command_loop:
    MOVI R0, prompt_text
    SYSCALL 1
    CALLI read_line
    MOVI R1, 0x500
    MOVI R2, help_word
    CALLI string_equal
    MOVI R3, 1
    CMP R0, R3
    JZ do_help
    ; string_equal advances both pointers; reset them for each comparison.
    MOVI R1, 0x500
    MOVI R2, quit_word
    CALLI string_equal
    MOVI R3, 1
    CMP R0, R3
    JZ do_quit
    MOVI R1, 0x500
    MOVI R2, echo_word
    CALLI string_equal
    MOVI R3, 1
    CMP R0, R3
    JZ do_echo
    JUMP do_unknown
do_help:
    MOVI R0, help_text
    SYSCALL 1
    JUMP command_loop
do_quit:
    MOVI R0, goodbye_text
    SYSCALL 1
    HALT
do_unknown:
    MOVI R0, unknown_text
    SYSCALL 1
    JUMP command_loop
do_echo:
    MOVI R0, text_prompt
    SYSCALL 1
    CALLI read_line
    MOVI R0, 0x500
    SYSCALL 1
    MOVI R0, 10
    SYSCALL 3
    JUMP command_loop

; R0/R1/R2 are scratch. Save at most 63 bytes, then a zero terminator.
read_line:
    MOVI R1, 0x500
read_loop:
    SYSCALL 2
    MOVI R2, 10
    CMP R0, R2
    JZ finish_input
    MOVI R2, 13
    CMP R0, R2
    JZ read_loop
    MOVI R2, 0x53F
    CMP R1, R2
    JZ read_loop
    STB [R1], R0
    INC R1
    JUMP read_loop
finish_input:
    MOVI R0, 0
    STB [R1], R0
    RET

; R1/R2: strings. R0: 1 if equal, else 0. Clobbers R0..R4 and zero flag.
string_equal:
    MOVI R4, 0
compare_loop:
    LDB R0, [R1]
    LDB R3, [R2]
    CMP R0, R3
    JNZ not_equal
    CMP R0, R4
    JZ equal
    INC R1
    INC R2
    JUMP compare_loop
equal:
    MOVI R0, 1
    RET
not_equal:
    MOVI R0, 0
    RET

prompt_text: .string "Command> "
help_text: .string "Commands:\nhelp: show help\nquit: quit\necho: read and print a line\n"
goodbye_text: .string "Goodbye from Handmade VM\n"
unknown_text: .string "Unknown command\n"
help_word: .string "help"
quit_word: .string "quit"
echo_word: .string "echo"
text_prompt: .string "Text> "

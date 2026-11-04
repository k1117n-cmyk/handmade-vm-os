; Day 42: code/strings below 0x500; input 0x500..0x53F; stored line 0x600..0x63F.
    MOVI R5, 0x600
    MOVI R0, 0
    STB [R5], R0
command_loop:
    MOVI R0, prompt_text
    SYSCALL 1
    CALLI read_line
    CALLI split_line
    MOVI R2, echo_word
    CALLI compare_command
    MOVI R3, 1
    CMP R0, R3
    JZ do_echo
    MOVI R2, set_word
    CALLI compare_command
    MOVI R3, 1
    CMP R0, R3
    JZ do_set
    ; show, help and quit accept only an empty argument.
    LDB R0, [R6]
    MOVI R3, 0
    CMP R0, R3
    JNZ do_unknown
    MOVI R2, show_word
    CALLI compare_command
    MOVI R3, 1
    CMP R0, R3
    JZ do_show
    MOVI R2, help_word
    CALLI compare_command
    MOVI R3, 1
    CMP R0, R3
    JZ do_help
    MOVI R2, quit_word
    CALLI compare_command
    MOVI R3, 1
    CMP R0, R3
    JZ do_quit
    JUMP do_unknown
do_set:
    MOVI R3, 0
    CMP R7, R3
    JNZ store_line
    MOVI R0, text_prompt
    SYSCALL 1
    CALLI read_line
    MOVI R6, 0x500
store_line:
    CALLI copy_line
    JUMP command_loop
do_show:
    MOVI R0, 0x600
    SYSCALL 1
    JUMP print_newline
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
    MOVI R3, 0
    CMP R7, R3
    JZ read_echo_text
print_argument:
    LDB R0, [R6]
    CMP R0, R3
    JZ print_newline
    SYSCALL 3
    INC R6
    JUMP print_argument
read_echo_text:
    MOVI R0, text_prompt
    SYSCALL 1
    CALLI read_line
    MOVI R0, 0x500
    SYSCALL 1
print_newline:
    MOVI R0, 10
    SYSCALL 3
    JUMP command_loop

; R6: source. Clobbers R0/R3/R5/R6 and flags; preserves R1/R2/R4/R7.
copy_line:
    MOVI R5, 0x600
    MOVI R3, 0
copy_byte:
    LDB R0, [R6]
    STB [R5], R0
    CMP R0, R3
    JZ copy_done
    INC R6
    INC R5
    JUMP copy_byte
copy_done:
    RET

; end_copy_line

; R6: argument pointer, R7: separator present. Clobbers R0/R2/R3 and flags.
split_line:
    MOVI R6, 0x500
    MOVI R7, 0
    MOVI R2, 32
    MOVI R3, 0
skip_leading:
    LDB R0, [R6]
    CMP R0, R2
    JNZ scan_command
    INC R6
    JUMP skip_leading
scan_command:
    CMP R0, R3
    JZ split_done
    CMP R0, R2
    JZ split_separator
    INC R6
    LDB R0, [R6]
    JUMP scan_command
split_separator:
    MOVI R0, 0
    STB [R6], R0
    MOVI R7, 1
skip_separator:
    INC R6
    LDB R0, [R6]
    CMP R0, R2
    JZ skip_separator
split_done:
    RET

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

; R2: comparison string. R1 is initialized internally from 0x500.
; R0: 1 if equal, else 0. Clobbers R0..R4 and zero flag.
compare_command:
    MOVI R1, 0x500
    MOVI R3, 32
find_command:
    LDB R0, [R1]
    CMP R0, R3
    JNZ string_equal
    INC R1
    JUMP find_command
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
help_text: .string "Commands:\nhelp: show help\nquit: quit\necho [text]: print text or read a line\nset [text]: replace the stored line or read a line\nshow: print the stored line\n"
goodbye_text: .string "Goodbye from Handmade VM\n"
unknown_text: .string "Unknown command\n"
help_word: .string "help"
quit_word: .string "quit"
echo_word: .string "echo"
text_prompt: .string "Text> "
set_word: .string "set"
show_word: .string "show"

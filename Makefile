CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic

VM := handmade-vm
SMALL_ASM := tools/small-asm
BOOT_MESSAGE_WRITER := tools/write-boot-message-bin
LINE_BUFFER_WRITER := tools/write-line-buffer-bin
WORD_COMMAND_WRITER := tools/write-word-command-bin
WORD_COMMAND_SUBROUTINE_WRITER := tools/write-word-command-subroutine-bin

ASM_PROGRAMS := \
	programs/hello.asm \
	programs/echo-char.asm \
	programs/prompt-echo.asm \
	programs/one-char-command.asm \
	programs/command-loop.asm

BIN_PROGRAMS := $(ASM_PROGRAMS:.asm=.bin)
DATA_PROGRAMS := programs/word-command-loop.bin programs/word-command-compare.bin programs/boot-message.bin programs/line-buffer.bin programs/word-command.bin programs/word-command-subroutine.bin

.PHONY: run-word-command-loop test-word-command-loop run-word-command-compare test-word-command-compare all programs run-boot-message run-line-buffer run-word-command run-word-command-subroutine run-command-loop test test-monitor test-boot-message test-line-buffer test-word-command test-word-command-quit test-word-command-unknown test-word-command-subroutine clean

all: $(VM) programs

$(VM): notes/vm.c
	$(CC) $(CFLAGS) $< -o $@

$(SMALL_ASM): tools/small-asm.c
	$(CC) $(CFLAGS) $< -o $@

$(BOOT_MESSAGE_WRITER): tools/write-boot-message-bin.c
	$(CC) $(CFLAGS) $< -o $@

$(LINE_BUFFER_WRITER): tools/write-line-buffer-bin.c
	$(CC) $(CFLAGS) $< -o $@

$(WORD_COMMAND_WRITER): tools/write-word-command-bin.c
	$(CC) $(CFLAGS) $< -o $@

$(WORD_COMMAND_SUBROUTINE_WRITER): tools/write-word-command-subroutine-bin.c
	$(CC) $(CFLAGS) $< -o $@

programs: $(BIN_PROGRAMS) $(DATA_PROGRAMS)

programs/%.bin: programs/%.asm $(SMALL_ASM)
	$(SMALL_ASM) $< $@

programs/boot-message.bin: $(BOOT_MESSAGE_WRITER)
	$(BOOT_MESSAGE_WRITER) $@

programs/line-buffer.bin: $(LINE_BUFFER_WRITER)
	$(LINE_BUFFER_WRITER) $@

programs/word-command.bin: $(WORD_COMMAND_WRITER)
	$(WORD_COMMAND_WRITER) $@

programs/word-command-subroutine.bin: $(WORD_COMMAND_SUBROUTINE_WRITER)
	$(WORD_COMMAND_SUBROUTINE_WRITER) $@

run-boot-message: $(VM) programs/boot-message.bin
	./$(VM) programs/boot-message.bin

run-line-buffer: $(VM) programs/line-buffer.bin
	./$(VM) programs/line-buffer.bin

run-word-command: $(VM) programs/word-command.bin
	./$(VM) programs/word-command.bin

run-word-command-subroutine: $(VM) programs/word-command-subroutine.bin
	./$(VM) programs/word-command-subroutine.bin

run-command-loop: $(VM) programs/command-loop.bin
	./$(VM) programs/command-loop.bin

test: $(VM) programs/command-loop.bin
	printf 'h \nx\nq\n' | ./$(VM) programs/command-loop.bin

test-monitor: $(VM) programs/hello.bin programs/line-buffer.bin
	printf 'programs/hello.bin\nprograms/line-buffer.bin\nhelp\nquit\n' | ./$(VM)

test-boot-message: $(VM) programs/boot-message.bin
	printf 'h\nq\n' | ./$(VM) programs/boot-message.bin

test-line-buffer: $(VM) programs/line-buffer.bin
	printf 'help\n' | ./$(VM) programs/line-buffer.bin

test-word-command: $(VM) programs/word-command.bin
	printf 'help\n' | ./$(VM) programs/word-command.bin

test-word-command-quit: $(VM) programs/word-command.bin
	printf 'quit\n' | ./$(VM) programs/word-command.bin

test-word-command-unknown: $(VM) programs/word-command.bin
	printf 'hello\n' | ./$(VM) programs/word-command.bin

test-word-command-subroutine: $(VM) programs/word-command-subroutine.bin
	printf 'help\n' | ./$(VM) programs/word-command-subroutine.bin
	printf 'quit\n' | ./$(VM) programs/word-command-subroutine.bin
	printf 'hello\n' | ./$(VM) programs/word-command-subroutine.bin

clean:
	rm -f tools/write-word-command-loop-bin tools/write-word-command-compare-bin $(VM) $(SMALL_ASM) $(BOOT_MESSAGE_WRITER) $(LINE_BUFFER_WRITER) $(WORD_COMMAND_WRITER) $(WORD_COMMAND_SUBROUTINE_WRITER)

tools/write-word-command-compare-bin: tools/write-word-command-compare-bin.c
	$(CC) $(CFLAGS) $< -o $@

programs/word-command-compare.bin: tools/write-word-command-compare-bin
	$< $@

run-word-command-compare: $(VM) programs/word-command-compare.bin
	./$(VM) programs/word-command-compare.bin

test-word-command-compare: $(VM) programs/word-command-compare.bin
	printf 'help\n' | ./$(VM) programs/word-command-compare.bin
	printf 'quit\n' | ./$(VM) programs/word-command-compare.bin
	printf 'hello\n' | ./$(VM) programs/word-command-compare.bin

tools/write-word-command-loop-bin: tools/write-word-command-loop-bin.c
	$(CC) $(CFLAGS) $< -o $@

programs/word-command-loop.bin: tools/write-word-command-loop-bin
	$< $@

run-word-command-loop: $(VM) programs/word-command-loop.bin
	./$(VM) programs/word-command-loop.bin

test-word-command-loop: $(VM) programs/word-command-loop.bin
	python3 tools/test-word-command-loop.py ./$(VM)

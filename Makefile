CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic

VM := handmade-vm
SMALL_ASM := tools/small-asm
BOOT_MESSAGE_WRITER := tools/write-boot-message-bin
LINE_BUFFER_WRITER := tools/write-line-buffer-bin

ASM_PROGRAMS := \
	programs/hello.asm \
	programs/echo-char.asm \
	programs/prompt-echo.asm \
	programs/one-char-command.asm \
	programs/command-loop.asm

BIN_PROGRAMS := $(ASM_PROGRAMS:.asm=.bin)
DATA_PROGRAMS := programs/boot-message.bin programs/line-buffer.bin

.PHONY: all programs run-boot-message run-line-buffer run-command-loop test test-monitor test-boot-message test-line-buffer clean

all: $(VM) programs

$(VM): notes/vm.c
	$(CC) $(CFLAGS) $< -o $@

$(SMALL_ASM): tools/small-asm.c
	$(CC) $(CFLAGS) $< -o $@

$(BOOT_MESSAGE_WRITER): tools/write-boot-message-bin.c
	$(CC) $(CFLAGS) $< -o $@

$(LINE_BUFFER_WRITER): tools/write-line-buffer-bin.c
	$(CC) $(CFLAGS) $< -o $@

programs: $(BIN_PROGRAMS) $(DATA_PROGRAMS)

programs/%.bin: programs/%.asm $(SMALL_ASM)
	$(SMALL_ASM) $< $@

programs/boot-message.bin: $(BOOT_MESSAGE_WRITER)
	$(BOOT_MESSAGE_WRITER) $@

programs/line-buffer.bin: $(LINE_BUFFER_WRITER)
	$(LINE_BUFFER_WRITER) $@

run-boot-message: $(VM) programs/boot-message.bin
	./$(VM) programs/boot-message.bin

run-line-buffer: $(VM) programs/line-buffer.bin
	./$(VM) programs/line-buffer.bin

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

clean:
	rm -f $(VM) $(SMALL_ASM) $(BOOT_MESSAGE_WRITER) $(LINE_BUFFER_WRITER)

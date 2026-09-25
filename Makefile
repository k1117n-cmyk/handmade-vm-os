CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic

VM := handmade-vm
SMALL_ASM := tools/small-asm

ASM_PROGRAMS := \
	programs/hello.asm \
	programs/echo-char.asm \
	programs/prompt-echo.asm \
	programs/one-char-command.asm \
	programs/command-loop.asm

BIN_PROGRAMS := $(ASM_PROGRAMS:.asm=.bin)

.PHONY: all programs run-command-loop test clean

all: $(VM) programs

$(VM): notes/vm.c
	$(CC) $(CFLAGS) $< -o $@

$(SMALL_ASM): tools/small-asm.c
	$(CC) $(CFLAGS) $< -o $@

programs: $(BIN_PROGRAMS)

programs/%.bin: programs/%.asm $(SMALL_ASM)
	$(SMALL_ASM) $< $@

run-command-loop: $(VM) programs/command-loop.bin
	./$(VM) programs/command-loop.bin

test: $(VM) programs/command-loop.bin
	printf 'h \nx\nq\n' | ./$(VM) programs/command-loop.bin

clean:
	rm -f $(VM) $(SMALL_ASM)

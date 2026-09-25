# Worklog

このファイルは、過去に実際に行った作業を残すためのログです。

次回再開に必要な短い情報は [HANDOFF.md](HANDOFF.md) に置きます。

## Day 26: prompt-echo sample program

追加:

```text
programs/prompt-echo.asm
programs/prompt-echo.bin
manual/specs/026-prompt-echo.md
manual/test-code-explanations/026-prompt-echo.md
```

## Day 28: command-loop sample program

追加:

```text
programs/command-loop.asm
programs/command-loop.bin
manual/specs/028-command-loop.md
manual/test-code-explanations/028-command-loop.md
manual/specs/029-syscall-print-char-raw.md
articles/2026-09-25-vm-command-loop-028-outline.md
```

更新:

```text
notes/vm.c
README.md
programs/README.md
manual/HANDOFF.md
manual/README.md
manual/instruction-types.md
manual/WORKLOG.md
articles/ROADMAP.md
```

確認結果:

```text
make test
```

```text
>H
>?
>CPU halted.
```

更新:

```text
README.md
programs/README.md
manual/HANDOFF.md
manual/README.md
manual/WORKLOG.md
ROADMAP.md
manual/NEXT_INSTRUCTION_GUIDELINES.md
```

確認結果:

```text
printf A | /tmp/handmade-vm programs/prompt-echo.bin
```

```text
>
A
CPU halted.
```

## Day 24: SYSCALL read_char

追加:

```text
manual/specs/024-syscall-read-char.md
notes/024-syscall-read-char-test.c
manual/test-code-explanations/024-syscall-read-char-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/HANDOFF.md
README.md
ROADMAP.md
manual/DOC_UPDATE_CHECKLIST.md
manual/NEXT_INSTRUCTION_GUIDELINES.md
notes/vm.c
```

確認結果:

```text
A
CPU halted.
SYSCALL read_char test passed.
```

`notes/vm.c`:

```text
A
VM flow complete.
CPU halted.
```

## Day 25: echo-char sample program

追加:

```text
programs/echo-char.asm
programs/echo-char.bin
manual/specs/025-echo-char.md
manual/test-code-explanations/025-echo-char.md
```

更新:

```text
README.md
programs/README.md
manual/HANDOFF.md
manual/README.md
```

確認結果:

```text
printf A | /tmp/handmade-vm programs/echo-char.bin
```

```text
A
CPU halted.
```

```text
A
CPU halted.
```

## Day 10: LDDI

追加:

```text
manual/specs/010-lddi.md
notes/010-lddi-test.c
manual/test-code-explanations/010-lddi-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R0=0x12345678
LDDI test passed.
```

`notes/vm.c`:

```text
A
CPU halted.
```

## Day 11: STDI

追加:

```text
manual/specs/011-stdi.md
notes/011-stdi-test.c
manual/test-code-explanations/011-stdi-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R0=0x12345678
memory[0x10..0x13]=12 34 56 78
STDI test passed.
```

`notes/vm.c`:

```text
A
CPU halted.
```

## Day 12: INC / DEC

追加:

```text
manual/specs/012-inc-dec.md
notes/012-inc-dec-test.c
manual/test-code-explanations/012-inc-dec-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R0=0x00000011
R1=0x0000000F
INC/DEC test passed.
```

`notes/vm.c`:

```text
A
CPU halted.
```

## Day 13: MOV

追加:

```text
manual/specs/013-mov.md
notes/013-mov-test.c
manual/test-code-explanations/013-mov-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R0=0x00012345
R2=0x00012345
MOV test passed.
```

`notes/vm.c`:

```text
A
CPU halted.
```

## Day 14: PUSH

追加:

```text
manual/specs/014-push.md
notes/014-push-test.c
manual/test-code-explanations/014-push-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R0=0x12345678
SP=0x000FFFFC
memory[SP..SP+3]=12 34 56 78
PUSH test passed.
```

`notes/vm.c`:

```text
A
CPU halted.
```

## Day 15: POP

追加:

```text
manual/specs/015-pop.md
notes/015-pop-test.c
manual/test-code-explanations/015-pop-test.md
```

更新:

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
R1=0x12345678
SP=0x00100000
POP test passed.
```

## Day 16-20: Arithmetic And Control Flow

対象:

```text
Day 16: ADD / SUB
Day 17: CMP / zero flag
Day 18: JUMP / JZ
Day 19: JNZ
Day 20: CALLI / RET
```

追加:

```text
manual/specs/016-add-sub.md
manual/specs/017-cmp.md
manual/specs/018-jump-jz.md
manual/specs/019-jnz.md
manual/specs/020-calli-ret.md

notes/016-add-sub-test.c
notes/017-cmp-test.c
notes/018-jump-jz-test.c
notes/019-jnz-test.c
notes/020-calli-ret-test.c

manual/test-code-explanations/016-add-sub-test.md
manual/test-code-explanations/017-cmp-test.md
manual/test-code-explanations/018-jump-jz-test.md
manual/test-code-explanations/019-jnz-test.md
manual/test-code-explanations/020-calli-ret-test.md
```

更新:

```text
HANDWRITING_GUIDE.md
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認結果:

```text
notes/*-test.c と notes/vm.c をすべて cc でコンパイル・実行
結果: all ok
```

`notes/vm.c`:

```text
A
VM flow complete.
CPU halted.
```

## Day 21: Binary Loader

追加:

```text
manual/specs/021-binary-loader.md
notes/021-binary-loader-test.c
manual/test-code-explanations/021-binary-loader-test.md
```

更新:

```text
README.md
manual/README.md
manual/instruction-types.md
notes/vm.c
```

`notes/vm.c` は、引数なしなら従来の内蔵テストプログラムを実行し、引数ありなら外部バイナリを `memory[0]` から読み込んで実行する。

確認結果:

```text
cc notes/021-binary-loader-test.c -o /tmp/021-binary-loader-test
/tmp/021-binary-loader-test

inst0=0x40000041
inst1=0x60000000
inst2=0x01000000
binary loader test passed.
```

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm /tmp/021-program.bin

A
CPU halted.
```

## programs/hello.bin

外部バイナリローダーをリポジトリ内の実ファイルで確認できるように追加。

```text
programs/README.md
programs/hello.bin
```

`programs/hello.bin` の中身:

```text
40 00 00 41
60 00 00 00
01 00 00 00
```

対応する命令:

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

確認結果:

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin

A
CPU halted.
```

## Day 22: Hello Binary Writer

`programs/hello.bin` を再生成できる小さな作成ツールを追加。

```text
tools/write-hello-bin.c
manual/specs/022-hello-binary-writer.md
manual/test-code-explanations/022-hello-binary-writer.md
```

更新:

```text
README.md
manual/README.md
programs/README.md
```

確認結果:

```text
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin

wrote programs/hello.bin
```

```text
xxd programs/hello.bin

00000000: 4000 0041 6000 0000 0100 0000            @..A`.......
```

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin

A
CPU halted.
```

## Day 23: Small Assembler

最小限のアセンブリ表記から外部バイナリを生成する小さな作成ツールを追加。

```text
tools/small-asm.c
programs/hello.asm
manual/specs/023-small-asm.md
manual/test-code-explanations/023-small-asm.md
```

更新:

```text
README.md
manual/README.md
manual/instruction-types.md
programs/README.md
```

`tools/small-asm.c` が最初に扱う命令:

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

確認結果:

```text
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm /tmp/hello-small-asm.bin

assembled 3 instructions to /tmp/hello-small-asm.bin
```

```text
xxd /tmp/hello-small-asm.bin

00000000: 4000 0041 6000 0000 0100 0000            @..A`.......
```

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm /tmp/hello-small-asm.bin

A
CPU halted.
```

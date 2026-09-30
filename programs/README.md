# Sample Programs

このディレクトリには、VMで読み込む小さな外部バイナリを置く。

## hello.asm / hello.bin

`hello.asm` と `hello.bin` は、外部バイナリローダー確認用の最小プログラム。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

byte列:

```text
40 00 00 41
60 00 00 00
01 00 00 00
```

再生成:

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
```

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm programs/hello.bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待出力:

```text
A
CPU halted.
```

## echo-char.asm / echo-char.bin

`echo-char.asm` と `echo-char.bin` は、入力系 `SYSCALL 2` の最小確認用プログラム。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

byte列:

```text
60 00 00 02
60 00 00 00
01 00 00 00
```

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/echo-char.asm programs/echo-char.bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待出力:

```text
A
CPU halted.
```

## prompt-echo.asm / prompt-echo.bin

`prompt-echo.asm` と `prompt-echo.bin` は、入力前に `>` を表示してから、入力した1文字をそのまま表示するサンプルプログラム。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
SYSCALL 0
HALT
```

byte列:

```text
40 00 00 3E
60 00 00 00
60 00 00 02
60 00 00 00
01 00 00 00
```

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/prompt-echo.asm programs/prompt-echo.bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/prompt-echo.bin
```

期待出力:

```text
>
A
CPU halted.
```

## one-char-command.asm / one-char-command.bin

`one-char-command.asm` と `one-char-command.bin` は、入力された1文字を `h`, `q`, その他に分けるサンプルプログラム。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
MOVI R1, 104
CMP R0, R1
JZ 0x1C
JNZ 0x28
MOVI R0, 72
SYSCALL 0
HALT
MOVI R1, 113
CMP R0, R1
JZ 0x44
JNZ 0x38
MOVI R0, 63
SYSCALL 0
HALT
HALT
```

`h` なら `H` を表示し、`q` ならそのまま停止し、それ以外なら `?` を表示する。

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/one-char-command.asm programs/one-char-command.bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf h | /tmp/handmade-vm programs/one-char-command.bin
printf q | /tmp/handmade-vm programs/one-char-command.bin
printf x | /tmp/handmade-vm programs/one-char-command.bin
```

期待出力:

```text
>
H
CPU halted.
```

```text
>
CPU halted.
```

```text
>
?
CPU halted.
```

## command-loop.asm / command-loop.bin

`command-loop.asm` と `command-loop.bin` は、入力された1文字を `h`, `q`, Enter, space, その他に分け、`q` 以外ではプロンプトへ戻るサンプルプログラム。

```asm
MOVI R0, 62
SYSCALL 3
SYSCALL 2
MOVI R1, 104
CMP R0, R1
JZ 0x5C
JNZ 0x1C
MOVI R1, 113
CMP R0, R1
JZ 0x74
JNZ 0x2C
MOVI R1, 10
CMP R0, R1
JZ 0x08
JNZ 0x3C
MOVI R1, 13
CMP R0, R1
JZ 0x08
JNZ 0x4C
MOVI R1, 32
CMP R0, R1
JZ 0x08
JNZ 0x68
MOVI R0, 72
SYSCALL 0
JUMP 0x00
MOVI R0, 63
SYSCALL 0
JUMP 0x00
HALT
```

`h` なら `H` を表示してプロンプトへ戻り、`q` なら停止し、Enter や space なら何も表示せず次の入力を待ち、それ以外なら `?` を表示してプロンプトへ戻る。

アセンブリ表記から再生成:

```sh
make programs/command-loop.bin
```

実行:

```sh
make
./handmade-vm programs/command-loop.bin
```

期待出力:

```text
>h
H
>x
?
>q
CPU halted.
```

自動確認する場合:

```sh
make test
```

自動確認では入力文字のechoが表示されないため、`>H`, `>?`, `>CPU halted.` のように見える。

## boot-message.bin

`boot-message.bin` は、命令列の後ろに0終端文字列を置き、起動メッセージ、help表示、終了メッセージを表示する1文字コマンドループのサンプルプログラム。

```text
0x00000000: MOVI R0, 0x100
0x00000004: SYSCALL 1
0x00000008: MOVI R0, 62
0x0000000C: SYSCALL 3
0x00000010: SYSCALL 2
...
0x00000064: MOVI R0, 0x140
0x00000068: SYSCALL 1
0x0000006C: JUMP 0x08
0x00000070: MOVI R0, 0x180
0x00000074: SYSCALL 1
0x00000078: HALT

0x00000100: "Welcome to Handmade VM\n\0"
0x00000140: "Commands:\nh: help\nq: quit\n\0"
0x00000180: "Goodbye from Handmade VM\n\0"
```

再生成:

```sh
make programs/boot-message.bin
```

実行:

```sh
make run-boot-message
```

期待出力:

```text
Welcome to Handmade VM
>h
Commands:
h: help
q: quit
>>q
Goodbye from Handmade VM
CPU halted.
```

## line-buffer.bin

`line-buffer.bin` は、Enterまでの入力をVM内メモリに保存し、0終端文字列として表示するサンプルプログラム。

```text
0x00000000: MOVI R0, 0x100
0x00000004: SYSCALL 1
0x00000008: MOVI R1, 0x180
0x0000000C: SYSCALL 2
0x00000010: MOVI R2, 10
0x00000014: CMP R0, R2
0x00000018: JZ 0x40
...
0x00000028: STB [R1], R0
0x0000002C: INC R1
0x00000030: MOVI R2, 0x1BF
0x00000034: CMP R1, R2
0x00000038: JZ 0x40
0x0000003C: JUMP 0x0C
0x00000040: MOVI R0, 0
0x00000044: STB [R1], R0
0x00000048: MOVI R0, 0x140
0x0000004C: SYSCALL 1
0x00000050: MOVI R0, 0x180
0x00000054: SYSCALL 1
0x00000058: MOVI R0, 10
0x0000005C: SYSCALL 3
0x00000060: HALT

0x00000100: "Type a word, then Enter\n>\0"
0x00000140: "You typed: \0"
0x00000180: 入力バッファ
```

再生成:

```sh
make programs/line-buffer.bin
```

実行:

```sh
make test-line-buffer
```

期待出力:

```text
Type a word, then Enter
>You typed: help
CPU halted.
```

## prompt-echo.asm / prompt-echo.bin の解説

`programs/prompt-echo.asm` は、入力前に `>` を表示してから、1文字を読んでそのまま表示するサンプルプログラムです。

`notes/026-...-test.c` ではありません。

Day 26 は、新しいVM命令を追加する回ではありません。Day 25 までに作った `SYSCALL 0`, `SYSCALL 2`, 外部バイナリローダー、`small-asm` を組み合わせて、少しOSのプロンプトらしい見た目を作る回です。

## プログラム

`programs/prompt-echo.asm` の中身は次の5命令です。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
SYSCALL 0
HALT
```

意味は次の通りです。

```text
MOVI R0, 62:
  R0へ `>` のASCIIコードを入れる

SYSCALL 0:
  R0の下位8bitを文字として表示する

SYSCALL 2:
  host標準入力から1 byte読み、R0へ入れる

SYSCALL 0:
  読んだ文字を表示する

HALT:
  VMを停止する
```

## echo-char.bin との違い

`programs/echo-char.asm` は、host標準入力から読んだ値をそのまま表示します。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

`programs/prompt-echo.asm` は、その前に `>` を表示します。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
SYSCALL 0
HALT
```

見た目の違いは小さいですが、意味は大きいです。入力を待つ前にプロンプトを出すことで、VM上のプログラムが少しOS風の入口に近づきます。

## small assembler で変換する

Day 23 で作った `tools/small-asm.c` は、次の命令を扱えます。

```asm
MOVI Rn, imm
SYSCALL imm
HALT
```

`prompt-echo.asm` はこの3種類だけで書けるので、`small-asm` を拡張せずに変換できます。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/prompt-echo.asm programs/prompt-echo.bin
```

出力:

```text
assembled 5 instructions to programs/prompt-echo.bin
```

## byte列

生成した `programs/prompt-echo.bin` を確認します。

```sh
xxd programs/prompt-echo.bin
```

出力:

```text
00000000: 4000 003e 6000 0000 6000 0002 6000 0000  @..>`...`...`...
00000010: 0100 0000                                ....
```

byte単位で見ると、次の5命令です。

```text
40 00 00 3E    MOVI R0, 62
60 00 00 00    SYSCALL 0
60 00 00 02    SYSCALL 2
60 00 00 00    SYSCALL 0
01 00 00 00    HALT
```

## VMで実行する

標準入力に `A` を渡して、VMで実行します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/prompt-echo.bin
```

期待する出力:

```text
>
A
CPU halted.
```

現在の `SYSCALL 0` は、1文字を表示したあとに改行も表示します。そのため、`>` と `A` は別の行に表示されます。

## 実行の流れ

実行の流れは次の通りです。

```text
1. VMがprograms/prompt-echo.binをmemory[0]から読み込む
2. PC=0からfetchを始める
3. MOVI R0, 62でR0に`>`のASCIIコードを入れる
4. SYSCALL 0で`>`を表示する
5. SYSCALL 2でhost標準入力から'A'を読む
6. R0に0x41が入る
7. SYSCALL 0でR0の下位8bitを表示する
8. HALTで停止する
```

このサンプルで確認しているのは、次の2点です。

```text
1. 新しい命令を追加しなくても、既存命令の組み合わせで見た目を変えられる
2. 入力前にプロンプトを表示することで、OS風プログラムの入口を作れる
```

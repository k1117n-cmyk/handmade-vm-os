## echo-char.asm / echo-char.bin の解説

`programs/echo-char.asm` は、入力系 `SYSCALL 2` を使う最小サンプルプログラムです。

`notes/025-...-test.c` ではありません。

Day 25 は、Day 24 で追加した `SYSCALL 2` を外部プログラムから使えることを確認する回です。そのため、Cコード本体は `notes/` ではなく、VMで読み込むプログラムとして `programs/` に置きます。

## プログラム

`programs/echo-char.asm` の中身は次の3命令です。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

意味は次の通りです。

```text
SYSCALL 2:
  host標準入力から1 byte読み、R0へ入れる

SYSCALL 0:
  R0の下位8bitを文字として表示する

HALT:
  VMを停止する
```

## hello.bin との違い

`programs/hello.asm` は、固定値 `65` を `R0` へ入れてから表示します。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

`programs/echo-char.asm` は、host標準入力から読んだ値を `R0` へ入れてから表示します。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

`printf A | ...` で実行すれば、どちらも `A` を表示できます。

ただし、値の出どころは違います。

```text
hello.asm:
  命令の中の即値 65

echo-char.asm:
  host標準入力から読んだ byte
```

## small assembler で変換する

Day 23 で作った `tools/small-asm.c` は、次の命令を扱えます。

```asm
MOVI Rn, imm
SYSCALL imm
HALT
```

`SYSCALL 2` は `SYSCALL imm` の形なので、そのまま変換できます。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/echo-char.asm programs/echo-char.bin
```

出力:

```text
assembled 3 instructions to programs/echo-char.bin
```

## byte列

生成した `programs/echo-char.bin` を確認します。

```sh
xxd programs/echo-char.bin
```

出力:

```text
00000000: 6000 0002 6000 0000 0100 0000            `...`.......
```

byte単位で見ると、次の3命令です。

```text
60 00 00 02    SYSCALL 2
60 00 00 00    SYSCALL 0
01 00 00 00    HALT
```

## VMで実行する

標準入力に `A` を渡して、VMで実行します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待する出力:

```text
A
CPU halted.
```

## 実行の流れ

実行の流れは次の通りです。

```text
1. VMがprograms/echo-char.binをmemory[0]から読み込む
2. PC=0からfetchを始める
3. SYSCALL 2でhost標準入力から'A'を読む
4. R0に0x41が入る
5. SYSCALL 0でR0の下位8bitを表示する
6. HALTで停止する
```

このサンプルで確認しているのは、次の2点です。

```text
1. SYSCALL 2を外部バイナリから呼べる
2. host標準入力から読んだ値を、そのままSYSCALL 0で表示できる
```

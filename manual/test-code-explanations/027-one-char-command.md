## one-char-command.asm / one-char-command.bin の解説

`programs/one-char-command.asm` は、入力された1文字を `h`, `q`, その他に分けるサンプルプログラムです。

Day 27 は、新しいVM命令を追加する回ではありません。すでに作った `CMP`, `JZ`, `JNZ`, `SYSCALL 2`, `SYSCALL 0` を組み合わせて、OS風プログラムの最初のコマンド判定を作る回です。

## プログラム

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

## 流れ

```text
1. `>` を表示する
2. host標準入力から1 byte読む
3. 読んだ文字と `h` を比較する
4. 同じなら `H` を表示して停止する
5. 違うなら `q` 判定へ進む
6. 読んだ文字と `q` を比較する
7. 同じならそのまま停止する
8. 違うなら `?` を表示して停止する
```

ここでは、本格的な `help` 文字列はまだ表示しません。文字列表示は後の回で `SYSCALL 1` とメモリ上の0終端文字列を使って扱います。

## 分岐先アドレス

このサンプルではラベルをまだ使わないため、分岐先を手で数える。

```text
JZ 0x1C:
  h と一致したとき、Hを表示する処理へ移動する

JNZ 0x28:
  h と一致しなかったとき、q判定へ移動する

JZ 0x44:
  q と一致したとき、そのまま停止する

JNZ 0x38:
  q と一致しなかったとき、?を表示する処理へ移動する
```

## small assembler で変換する

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/one-char-command.asm programs/one-char-command.bin
```

出力:

```text
assembled 18 instructions to programs/one-char-command.bin
```

## byte列

```sh
xxd programs/one-char-command.bin
```

出力:

```text
00000000: 4000 003e 6000 0000 6000 0002 4010 0068  @..>`...`...@..h
00000010: 2401 0000 6a00 001c 6b00 0028 4000 0048  $...j...k..(@..H
00000020: 6000 0000 0100 0000 4010 0071 2401 0000  `.......@..q$...
00000030: 6a00 0044 6b00 0038 4000 003f 6000 0000  j..Dk..8@..?`...
00000040: 0100 0000 0100 0000                      ........
```

## VMで実行する

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf h | /tmp/handmade-vm programs/one-char-command.bin
printf q | /tmp/handmade-vm programs/one-char-command.bin
printf x | /tmp/handmade-vm programs/one-char-command.bin
```

期待する出力:

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

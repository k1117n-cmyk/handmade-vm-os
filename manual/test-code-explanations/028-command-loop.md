## command-loop.asm / command-loop.bin の解説

`programs/command-loop.asm` は、入力された1文字を `h`, `q`, Enter, space, その他に分け、`q` 以外ではプロンプトへ戻るサンプルプログラムです。

Day 28 は、新しいCPU命令を追加する回ではありません。すでに作った `CMP`, `JUMP`, `JZ`, `JNZ`, `SYSCALL 2`, `SYSCALL 0`, `SYSCALL 3` を組み合わせて、OS風プログラムを「1回で終わる処理」から「入力を待ち続ける処理」へ進めます。

## プログラム

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

## 流れ

```text
1. `>` を改行なしで表示する
2. host標準入力から1 byte読む
3. 読んだ文字と `h` を比較する
4. 同じなら `H` を表示して、先頭へ戻る
5. 違うなら `q` 判定へ進む
6. 読んだ文字と `q` を比較する
7. 同じなら停止する
8. 違うなら LF / CR / space と比較する
9. LF / CR / space なら何も表示せず、次の入力読み取りへ戻る
10. それ以外なら `?` を表示して、先頭へ戻る
```

Day 27 の `one-char-command` では、各分岐の最後に `HALT` していた。Day 28 では、`h` とその他の処理の最後を `JUMP 0x00` に置き換える。

## 分岐先アドレス

このサンプルではラベルをまだ使わないため、分岐先を手で数える。

```text
JZ 0x5C:
  h と一致したとき、Hを表示する処理へ移動する

JNZ 0x1C:
  h と一致しなかったとき、q判定へ移動する

JZ 0x74:
  q と一致したとき、そのまま停止する

JNZ 0x2C:
  q と一致しなかったとき、LF判定へ移動する

JZ 0x08:
  LF または CR と一致したとき、何も表示せず次の入力読み取りへ戻る

JNZ 0x4C:
  CR とも一致しなかったとき、space判定へ移動する

JZ 0x08:
  space と一致したとき、何も表示せず次の入力読み取りへ戻る

JNZ 0x68:
  space とも一致しなかったとき、?を表示する処理へ移動する

JUMP 0x00:
  h またはその他の処理後、プロンプト表示へ戻る
```

## small assembler で変換する

```sh
make programs/command-loop.bin
```

出力:

```text
assembled 30 instructions to programs/command-loop.bin
```

## byte列

```sh
xxd programs/command-loop.bin
```

出力:

```text
00000000: 4000 003e 6000 0003 6000 0002 4010 0068  @..>`...`...@..h
00000010: 2401 0000 6a00 005c 6b00 001c 4010 0071  $...j..\k...@..q
00000020: 2401 0000 6a00 0074 6b00 002c 4010 000a  $...j..tk..,@...
00000030: 2401 0000 6a00 0008 6b00 003c 4010 000d  $...j...k..<@...
00000040: 2401 0000 6a00 0008 6b00 004c 4010 0020  $...j...k..L@.. 
00000050: 2401 0000 6a00 0008 6b00 0068 4000 0048  $...j...k..h@..H
00000060: 6000 0000 6800 0000 4000 003f 6000 0000  `...h...@..?`...
00000070: 6800 0000 0100 0000                      h.......
```

## VMで実行する

```sh
make
./handmade-vm programs/command-loop.bin
```

期待する出力:

```text
>h
H
>x
?
>q
CPU halted.
```

`h` では `H` を表示してプロンプトへ戻り、`x` では `?` を表示してプロンプトへ戻る。最後に `q` を読むと `HALT` に到達して停止する。Enterによって届く LF / CR や、うっかり入力したspaceは次の入力読み取りへ戻すため、余分な `?` や二重のプロンプトは表示されない。

同じ入力を自動で流す場合は、次も使える。

```sh
make test
```

pipeで入力を渡す場合は、入力文字のechoが表示されない。そのため、自動確認の見え方は次のようになる。

```text
>H
>?
>CPU halted.
```

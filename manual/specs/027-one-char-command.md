## Day 27: one-char-command sample program

```text
名前: one-char-command
分類: sample program
目的: 1文字入力を読み、h / q / その他で分岐する
命令長: なし。プログラム全体としては18命令、72 byte
bit配置: 各命令は既存の32bit命令フォーマットに従う
読むレジスタ: CMPでR0/R1、SYSCALL 0でR0
書くレジスタ: MOVIでR0/R1、SYSCALL 2でR0
読むメモリ: fetch時にmemory[PC]から命令を読む
書くメモリ: なし
PCの変化: 各命令のfetch時に +4。JZ/JNZ成立時はPC = imm
条件フラグの変化: CMPでzero_flagを更新する
エラー時: SYSCALL 2でEOFなら "input EOF" を表示して停止
手作りテスト: programs/one-char-command.asm から programs/one-char-command.bin を作る
成功条件: hならHを表示、qならそのまま停止、それ以外なら?を表示する
```

Day 26 では、入力前に `>` を表示してから1文字echoする `prompt-echo` を作った。

Day 27 では、新しいVM命令は追加しない。第3回で追加済みの `CMP`, `JZ`, `JNZ` を、VM上で動く外部プログラムのコマンド判定に使う。

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

## 分岐先

この回ではラベルをまだ使わず、分岐先を即値アドレスで書く。

```text
0x1C: h の処理。Hを表示して停止する
0x28: q 判定へ進む
0x38: その他の処理。?を表示して停止する
0x44: q の処理。そのまま停止する
```

`h` のASCIIコードは `104`、`q` は `113`、`H` は `72`、`?` は `63`。

## 命令列

```text
0x00000000: 40 00 00 3E    MOVI R0, 62
0x00000004: 60 00 00 00    SYSCALL 0
0x00000008: 60 00 00 02    SYSCALL 2
0x0000000C: 40 10 00 68    MOVI R1, 104
0x00000010: 24 01 00 00    CMP R0, R1
0x00000014: 6A 00 00 1C    JZ 0x1C
0x00000018: 6B 00 00 28    JNZ 0x28
0x0000001C: 40 00 00 48    MOVI R0, 72
0x00000020: 60 00 00 00    SYSCALL 0
0x00000024: 01 00 00 00    HALT
0x00000028: 40 10 00 71    MOVI R1, 113
0x0000002C: 24 01 00 00    CMP R0, R1
0x00000030: 6A 00 00 44    JZ 0x44
0x00000034: 6B 00 00 38    JNZ 0x38
0x00000038: 40 00 00 3F    MOVI R0, 63
0x0000003C: 60 00 00 00    SYSCALL 0
0x00000040: 01 00 00 00    HALT
0x00000044: 01 00 00 00    HALT
```

## 再生成

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/one-char-command.asm programs/one-char-command.bin
```

`tools/small-asm.c` は、このサンプル用に `CMP`, `JUMP`, `JZ`, `JNZ` も扱えるようにする。

## 実行

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

現在の `SYSCALL 0` は、1文字を表示したあとに改行も表示する。そのため、`>` と判定結果は別の行に表示される。

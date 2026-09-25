## Day 28: command-loop sample program

```text
名前: command-loop
分類: sample program
目的: 1文字入力を読み、h / q / Enter / space / その他で分岐し、q以外ではプロンプトへ戻る
命令長: なし。プログラム全体としては30命令、120 byte
bit配置: 各命令は既存の32bit命令フォーマットに従う
読むレジスタ: CMPでR0/R1、SYSCALL 0/3でR0
書くレジスタ: MOVIでR0/R1、SYSCALL 2でR0
読むメモリ: fetch時にmemory[PC]から命令を読む
書くメモリ: なし
PCの変化: 各命令のfetch時に +4。JUMP/JZ/JNZ成立時はPC = imm
条件フラグの変化: CMPでzero_flagを更新する
エラー時: SYSCALL 2でEOFなら "input EOF" を表示して停止
手作りテスト: programs/command-loop.asm から programs/command-loop.bin を作る
成功条件: hならHを表示してプロンプトへ戻り、qなら停止し、Enterやspaceなら何も表示せず次の入力を待ち、それ以外なら?を表示してプロンプトへ戻る
```

Day 27 では、入力された1文字を `h`, `q`, その他に分ける `one-char-command` を作った。

Day 28 では、新しいVM命令は追加しない。Day 27 の判定処理に `JUMP 0x00` を足して、`q` 以外では先頭のプロンプト表示へ戻る。

プロンプト表示には、改行なしで1文字を表示する `SYSCALL 3` を使う。

手入力では、ターミナルから送られる Enter やスペースも1 byteとして読まれる。そのため、この版では LF (`10`), CR (`13`), space (`32`) を何も表示せずに無視し、次の入力読み取りへ戻る。

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

## 分岐先

この回でもラベルをまだ使わず、分岐先を即値アドレスで書く。

```text
0x00: プロンプト表示から始まるループ先頭
0x08: Enterを無視したあとに戻る入力読み取り位置
0x1C: q 判定へ進む
0x2C: LF判定へ進む
0x3C: CR判定へ進む
0x4C: space判定へ進む
0x5C: h の処理。Hを表示して 0x00 へ戻る
0x68: その他の処理。?を表示して 0x00 へ戻る
0x74: q の処理。そのまま停止する
```

`h` のASCIIコードは `104`、`q` は `113`、LF は `10`、CR は `13`、space は `32`、`H` は `72`、`?` は `63`。

## 命令列

```text
0x00000000: 40 00 00 3E    MOVI R0, 62
0x00000004: 60 00 00 03    SYSCALL 3
0x00000008: 60 00 00 02    SYSCALL 2
0x0000000C: 40 10 00 68    MOVI R1, 104
0x00000010: 24 01 00 00    CMP R0, R1
0x00000014: 6A 00 00 4C    JZ 0x4C
0x00000018: 6B 00 00 1C    JNZ 0x1C
0x0000001C: 40 10 00 71    MOVI R1, 113
0x00000020: 24 01 00 00    CMP R0, R1
0x00000024: 6A 00 00 64    JZ 0x64
0x00000028: 6B 00 00 2C    JNZ 0x2C
0x0000002C: 40 10 00 0A    MOVI R1, 10
0x00000030: 24 01 00 00    CMP R0, R1
0x00000034: 6A 00 00 08    JZ 0x08
0x00000038: 6B 00 00 3C    JNZ 0x3C
0x0000003C: 40 10 00 0D    MOVI R1, 13
0x00000040: 24 01 00 00    CMP R0, R1
0x00000044: 6A 00 00 08    JZ 0x08
0x00000048: 6B 00 00 58    JNZ 0x58
0x0000004C: 40 00 00 48    MOVI R0, 72
0x00000050: 60 00 00 00    SYSCALL 0
0x00000054: 68 00 00 00    JUMP 0x00
0x00000058: 40 00 00 3F    MOVI R0, 63
0x0000005C: 60 00 00 00    SYSCALL 0
0x00000060: 68 00 00 00    JUMP 0x00
0x00000064: 01 00 00 00    HALT
```

## 再生成

```sh
make programs/command-loop.bin
```

## 実行

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

`SYSCALL 3` は、1文字を表示したあとに改行しない。そのため、プロンプト `>` と入力文字が同じ行に表示される。

上の例では、入力した `h`, `x`, `q` はターミナル側のechoで表示される。`h` や `x` のあとにEnterを押すと、そのEnterもVMへ届くが、LF/CR判定で無視して次の読み取りへ戻る。

自動確認する場合は、次のように `h`, `x`, `q` をまとめて渡す。

```sh
make test
```

`make test` はpipeで入力を渡すため、ターミナル側の入力echoは表示されない。そのため、出力は次のように見える。

```text
>H
>?
>CPU halted.
```

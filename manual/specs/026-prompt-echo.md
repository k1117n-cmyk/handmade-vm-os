## Day 26: prompt-echo sample program

```text
名前: prompt-echo
分類: sample program
目的: `>` を表示してからhost標準入力から1 byte読み、同じ文字をhost標準出力へ表示する
命令長: なし。プログラム全体としては5命令、20 byte
bit配置: 各命令は既存の32bit命令フォーマットに従う
読むレジスタ: SYSCALL 0でR0
書くレジスタ: MOVIでR0、SYSCALL 2でR0
読むメモリ: fetch時にmemory[PC]から命令を読む
書くメモリ: なし
PCの変化: 各命令のfetch時に +4
条件フラグの変化: なし
エラー時: SYSCALL 2でEOFなら "input EOF" を表示して停止
手作りテスト: programs/prompt-echo.asm から programs/prompt-echo.bin を作る
成功条件: printf A | /tmp/handmade-vm programs/prompt-echo.bin で >、A、CPU halted. が表示される
```

Day 25 では、`SYSCALL 2` で読んだ1文字をそのまま表示する `echo-char` を作った。

Day 26 では、新しいVM命令を追加しない。既存の `MOVI`, `SYSCALL 0`, `SYSCALL 2`, `HALT` を組み合わせて、入力前に `>` を表示する。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
SYSCALL 0
HALT
```

## 命令列

対応する命令値は次の通り。

```text
MOVI R0, 62  -> 0x4000003E
SYSCALL 0    -> 0x60000000
SYSCALL 2    -> 0x60000002
SYSCALL 0    -> 0x60000000
HALT         -> 0x01000000
```

このVMの命令はbig-endianでメモリへ置くので、ファイル上のbyte列は次の形になる。

```text
0x4000003E -> 40 00 00 3E
0x60000000 -> 60 00 00 00
0x60000002 -> 60 00 00 02
0x60000000 -> 60 00 00 00
0x01000000 -> 01 00 00 00
```

## `>` の表示

`>` のASCIIコードは10進数で `62`、16進数で `0x3E`。

```asm
MOVI R0, 62
SYSCALL 0
```

この2命令で、`R0` に `>` の文字コードを入れて、host標準出力へ表示する。

## 入力とecho

続く2命令で、host標準入力から1 byteを読み、そのまま表示する。

```asm
SYSCALL 2
SYSCALL 0
```

`SYSCALL 2` はhost標準入力から読んだ1 byteを `R0` へ入れる。直後の `SYSCALL 0` は、その `R0` の下位8bitを文字として表示する。

## 再生成

`tools/small-asm.c` は `MOVI Rn, imm`, `SYSCALL imm`, `HALT` を扱えるので、`prompt-echo.asm` はそのまま変換できる。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/prompt-echo.asm programs/prompt-echo.bin
```

生成後にbyte列を確認する。

```sh
xxd programs/prompt-echo.bin
```

期待するbyte列:

```text
00000000: 4000 003e 6000 0000 6000 0002 6000 0000  @..>`...`...`...
00000010: 0100 0000                                ....
```

## 実行

標準入力から `A` を渡して実行する。

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

現在の `SYSCALL 0` は、1文字を表示したあとに改行も表示する。そのため、`>` と `A` は別の行に表示される。

## 位置づけ

`echo-char.bin` は、入力した1文字をそのまま表示する最小プログラム。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

`prompt-echo.bin` は、その前に `>` を表示する。

```asm
MOVI R0, 62
SYSCALL 0
SYSCALL 2
SYSCALL 0
HALT
```

新しい命令は増えていないが、VM上の外部プログラムが少しOSのプロンプトらしい見た目に近づく。

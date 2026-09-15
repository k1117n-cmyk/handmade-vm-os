## Day 25: echo-char sample program

```text
名前: echo-char
分類: sample program
目的: host標準入力から1 byte読み、同じ文字をhost標準出力へ表示する
命令長: なし。プログラム全体としては3命令、12 byte
bit配置: 各命令は既存の32bit命令フォーマットに従う
読むレジスタ: SYSCALL 0でR0
書くレジスタ: SYSCALL 2でR0
読むメモリ: fetch時にmemory[PC]から命令を読む
書くメモリ: なし
PCの変化: 各命令のfetch時に +4
条件フラグの変化: なし
エラー時: SYSCALL 2でEOFなら "input EOF" を表示して停止
手作りテスト: programs/echo-char.asm から programs/echo-char.bin を作る
成功条件: printf A | /tmp/handmade-vm programs/echo-char.bin で A と CPU halted. が表示される
```

Day 24 では、入力系 `SYSCALL 2` を追加した。

Day 25 では、その `SYSCALL 2` を使う最小の外部プログラムを `programs/` に置く。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

## 命令列

対応する命令値は次の通り。

```text
SYSCALL 2    -> 0x60000002
SYSCALL 0    -> 0x60000000
HALT         -> 0x01000000
```

このVMの命令はbig-endianでメモリへ置くので、ファイル上のbyte列は次の形になる。

```text
0x60000002 -> 60 00 00 02
0x60000000 -> 60 00 00 00
0x01000000 -> 01 00 00 00
```

## 再生成

`tools/small-asm.c` は `SYSCALL imm` を扱えるので、`SYSCALL 2` もそのまま変換できる。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/echo-char.asm programs/echo-char.bin
```

生成後にbyte列を確認する。

```sh
xxd programs/echo-char.bin
```

期待するbyte列:

```text
00000000: 6000 0002 6000 0000 0100 0000            `...`.......
```

## 実行

標準入力から `A` を渡して実行する。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待する出力:

```text
A
CPU halted.
```

## 位置づけ

`hello.bin` は、固定値を表示する最小プログラム。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

`echo-char.bin` は、host標準入力から読んだ値を表示する最小プログラム。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

見た目の出力はどちらも `A` にできるが、値の出どころが違う。

```text
hello.bin:
  MOVIでVM内に固定値を置く

echo-char.bin:
  SYSCALL 2でhost標準入力から値を受け取る
```

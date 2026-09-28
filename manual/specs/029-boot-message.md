## Day 29: boot-message sample

```text
名前: boot-message.bin
分類: sample program
目的: 外部バイナリ内に0終端文字列を配置し、起動メッセージ、help表示、終了メッセージを表示する
命令長: 4 byte固定
使う命令:
  MOVI R0, address
  SYSCALL 1
  SYSCALL 2
  SYSCALL 3
  CMP
  JUMP
  JZ
  JNZ
  HALT
読むレジスタ: R0
書くレジスタ: R0, R1
読むメモリ: memory[R0] から 0 byte まで
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
成功条件: 起動メッセージを表示して入力待ちになり、hでhelp表示、qで終了メッセージを表示して停止する
```

## プログラムの形

`programs/boot-message.bin` は、起動メッセージを表示したあと、1文字コマンドループへ入る。

```text
0x00000000: 40 00 01 00    MOVI R0, 0x100
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 40 00 00 3E    MOVI R0, 62
0x0000000C: 60 00 00 03    SYSCALL 3
0x00000010: 60 00 00 02    SYSCALL 2
...
0x00000064: 40 00 01 40    MOVI R0, 0x140
0x00000068: 60 00 00 01    SYSCALL 1
0x0000006C: 68 00 00 08    JUMP 0x08
0x00000070: 40 00 01 80    MOVI R0, 0x180
0x00000074: 60 00 00 01    SYSCALL 1
0x00000078: 01 00 00 00    HALT

0x00000100: "Welcome to Handmade VM\n\0"
0x00000140: "Commands:\nh: help\nq: quit\n\0"
0x00000180: "Goodbye from Handmade VM\n\0"
```

`PC` は命令列を読む。`SYSCALL 1` は `R0` の値を文字列の先頭アドレスとして使い、0 byte が出るまで表示する。

起動直後は `0x100` の文字列を表示する。そのあと `0x08` へ戻るループに入り、`h` なら `0x140` のhelp文字列を表示し、`q` なら `0x180` の終了メッセージを表示して停止する。

```text
PCで読む => 命令
R0で読む => 文字列データ
```

## 生成ツール

このサンプルは、まだ小アセンブラのデータ定義を増やさず、専用のwriterで生成する。

```sh
cc tools/write-boot-message-bin.c -o /tmp/write-boot-message-bin
/tmp/write-boot-message-bin
```

`tools/write-boot-message-bin.c` は、命令をbig-endianで書き込み、`fseek` で `0x100`, `0x140`, `0x180` へ移動して文字列データを書き込む。

## 実行

```sh
make run-boot-message
```

手動確認では、起動後に `h`、Enter、`q`、Enter の順に入力する。

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

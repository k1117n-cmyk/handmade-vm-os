## Day 7: SYSCALL print_string

```text
名前: SYSCALL print_string
分類: syscall
目的: VM内メモリの0終端文字列をhost端末へ出す
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number = 1
読むレジスタ: R0
書くレジスタ: なし
読むメモリ: memory[R0] から 0 byte まで
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: 未実装syscallなら番号を表示して停止
手作りテスト: SYSCALL 1 -> 0x60000001
成功条件: R0が指す0終端文字列が表示される
```

テストプログラム:

```text
MOVI R0, 0x10
SYSCALL 1
HALT
```

メモリ配置:

```text
0x00000000: 40 00 00 10    MOVI R0, 0x10
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 01 00 00 00    HALT

0x00000010: 48 65 6C 6C 6F 20 66 72
0x00000018: 6F 6D 20 48 61 6E 64 6D
0x00000020: 61 64 65 20 4F 53 0A 00
```

期待する出力:

```text
Hello from Handmade OS
CPU halted.
```

学習ポイント:

```text
PCは命令を読む場所を指す。
R0はデータの場所を指す。
SYSCALL 1はR0を使ってデータを読む。
```

今回のPCの進み方:

```text
PC=0x00
memory[0x00..0x03] = 40 00 00 10
=> MOVI R0, 0x10

PC=0x04
memory[0x04..0x07] = 60 00 00 01
=> SYSCALL 1

PC=0x08
memory[0x08..0x0B] = 01 00 00 00
=> HALT
```

SYSCALL 1が読む場所:

```text
R0 = 0x10

memory[0x10] = 'H'
memory[0x11] = 'e'
memory[0x12] = 'l'
...
memory[0x26] = '\n'
memory[0x27] = 0x00
```

整理:

```text
PCで読む   => 命令として読む
R0で読む   => データとして読む
```

ASCIIコードの見方:

```text
'H'  = 0x48
'e'  = 0x65
'l'  = 0x6C
'o'  = 0x6F
' '  = 0x20
'\n' = 0x0A
0x00 = 文字列の終わり
```

Cコードで:

```c
memory[0x10] = 'H';
```

と書くのは、今回の文字コードでは:

```c
memory[0x10] = 0x48;
```

とほぼ同じ意味。

文字列全体を16進数で見る:

```sh
printf "Hello from Handmade OS\n" | hexdump -C
```

## ASCIIの見方

• man ascii 以外にも見方はいくつかあります。

  一番手軽なのは printf です。

```sh

  printf "%x\n" "'H"

  48
```

  10進数も見たいなら:
  
```sh
  printf "%d\n" "'H"

  72
```

  逆に、16進数 48 が何の文字か見たいなら:

```sh
  printf "\x48\n"

  H
```

出力の見方:

```text
00000000  48 65 6c 6c 6f 20 66 72  6f 6d 20 48 61 6e 64 6d  |Hello from Handm|
00000010  61 64 65 20 4f 53 0a                              |ade OS.|
```

左側は先頭からの位置。
中央はbyteを16進数で表示したもの。
右側は表示できるbyteを文字として見たもの。

0終端まで含めて見る:

```sh
printf "Hello from Handmade OS\n\0" | hexdump -C
```

`hexdump -C` の `-C` はC言語の意味ではない。
標準的な見やすい形式で、16進数とASCII表示を並べて出す指定。

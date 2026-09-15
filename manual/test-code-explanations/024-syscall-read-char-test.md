## 024-syscall-read-char-test.c の解説

`024-syscall-read-char-test.c` は、小さな自作VM上で `SYSCALL 2`、つまり「host標準入力から1 byte読んで `R0` へ入れる処理」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
SYSCALL 2          ; host標準入力から1 byte読んでR0へ入れる
SYSCALL 0          ; R0の下位8bitを文字として出力
HALT
```

このテストでは、実行時に標準入力へ `A` を渡します。

```sh
printf A | /tmp/024-syscall-read-char-test
```

期待される流れは次の通りです。

```text
host stdinの'A'
  -> SYSCALL 2
  -> R0 = 0x41
  -> SYSCALL 0
  -> host stdoutへ'A'を表示
```

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 60 00 00 02    SYSCALL 2
0x00000004: 60 00 00 00    SYSCALL 0
0x00000008: 01 00 00 00    HALT
```

命令は4 byte固定長なので、`pc` は `0x00`, `0x04`, `0x08` と進みます。

## SYSCALL 2

`SYSCALL 2` の命令値は `0x60000002` です。

分解すると次のようになります。

```text
type = 6
op   = 0
imm  = 2
```

`imm` が syscall 番号として使われています。

```c
} else if (imm == 2) {
    int ch = getchar();

    if (ch == EOF) {
        printf("input EOF\n");
        running = false;
    } else {
        regs[0] = (uint8_t)ch;
    }
}
```

`getchar()` はhost側の標準入力から1文字を読みます。

今回は `printf A | ...` で実行するので、`getchar()` は `'A'` を読みます。

ASCIIでは `'A'` は `0x41` なので、実行後は次の状態になります。

```text
R0 = 0x00000041
```

## SYSCALL 0

次の命令は `SYSCALL 0` です。

```c
putchar(regs[0] & 0xFF);
putchar('\n');
```

`R0` の下位8bitを文字として表示します。

`R0` には直前の `SYSCALL 2` で読んだ `0x41` が入っているので、表示される文字は `A` になります。

## EOF

標準入力から読めるbyteがない場合、`getchar()` は `EOF` を返します。

この段階では、EOFをVMプログラムへ返すのではなく、VM側で停止します。

```text
input EOF
```

行編集やEOF時の分岐は、あとでOS風プログラムが必要になった時点で考えます。

## 実行結果

期待される出力は次の通りです。

```text
A
CPU halted.
SYSCALL read_char test passed.
```

このテストで確認しているのは、次の2点です。

```text
1. SYSCALL 2でhost標準入力から1 byte読める
2. 読んだ値がR0へ入り、SYSCALL 0で同じ文字を表示できる
```

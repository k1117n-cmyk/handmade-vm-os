## Day 24: SYSCALL read_char

```text
名前: SYSCALL read_char
分類: syscall
目的: host標準入力から1 byte読み、VM内のR0へ入れる
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number = 2
読むレジスタ: なし
書くレジスタ: R0
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: EOFなら "input EOF" を表示して停止
手作りテスト: SYSCALL 2 -> 0x60000002
成功条件: host標準入力から読んだ1 byteがR0に入る
```

テストプログラム:

```text
SYSCALL 2
SYSCALL 0
HALT
```

期待する入力:

```text
A
```

期待する出力:

```text
A
CPU halted.
```

学習ポイント:

```text
SYSCALL 0はVM内のR0をhost端末へ出す。
SYSCALL 2はhost端末から読んだ値をVM内のR0へ入れる。
```

この2つをつなぐと、最小のechoになります。

```text
host stdin
  -> SYSCALL 2
  -> R0
  -> SYSCALL 0
  -> host stdout
```

最初は1 byteだけ読む。

行全体の入力、バックスペース、Enter判定、プロンプト表示はまだ扱わない。

## EOFの扱い

host標準入力から読めるbyteがない場合はEOFとする。

この段階では、EOFをVMプログラムへ戻り値として渡すのではなく、VM側で次のメッセージを表示して停止する。

```text
input EOF
```

理由:

```text
最初の成功条件を「1文字読める」に集中するため。
EOFをR0へ入れて分岐する処理は、あとでOS風プログラムが必要になった時点で考える。
```

## Cコードでの対応

host側では、まず `getchar()` で1 byte読む。

```c
int ch = getchar();
```

読めた場合:

```c
vm->regs[0] = (uint8_t)ch;
```

EOFの場合:

```c
printf("input EOF\n");
vm->running = false;
```

## 確認

個別テスト `notes/024-syscall-read-char-test.c` で確認する。

確認コマンドの形:

```sh
cc notes/024-syscall-read-char-test.c -o /tmp/024-syscall-read-char-test
printf A | /tmp/024-syscall-read-char-test
```

期待出力:

```text
A
CPU halted.
SYSCALL read_char test passed.
```

統合VMでは、同じ命令列を外部バイナリとして読み込んで確認する。

```text
60 00 00 02    SYSCALL 2
60 00 00 00    SYSCALL 0
01 00 00 00    HALT
```

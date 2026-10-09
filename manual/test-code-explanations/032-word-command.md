## word-command.bin の解説

`word-command.bin` は、入力バッファに入った0終端文字列を固定コマンドと比較するサンプルです。

前回の `line-buffer.bin` では、入力を保存してそのまま表示するところまで進みました。今回は、保存した文字列を `help` / `quit` と比べて、処理を分岐します。

## 何を確認するか

確認したいことは、次の3つです。

```text
1. Enterまでの入力を0終端文字列として保存する
2. LDBで入力バッファから1 byteずつ読む
3. CMPとJNZで、複数文字コマンドを分岐する
```

## 入力バッファ

入力バッファは `0x500` から始まる。

```text
0x500: h
0x501: e
0x502: l
0x503: p
0x504: 0x00
```

最後の `0x00` まで比較することで、完全一致だけをコマンドとして扱う。

たとえば `helpful` は先頭4文字が `help` でも、その次が `0x00` ではないため `help` とは判定されない。

## helpとの比較

比較は、文字ごとに次の形で行う。

```text
MOVI R1, 0x500
LDB R0, [R1]
MOVI R2, 104
CMP R0, R2
JNZ compare_quit
```

`104` は文字 `h` のASCIIコード。

一致すれば次の文字へ進む。一致しなければ、`help` ではないため `quit` の比較へ進む。

## quitとの比較

`help` と一致しなかった場合は、同じ形で `quit` と比較する。

```text
q
u
i
t
0x00
```

途中で1文字でも違えば `Unknown command` を表示する。

## 実行結果

```sh
make test-word-command
```

```text
Command> Commands:
help: show help
quit: quit
CPU halted.
```

```sh
make test-word-command-quit
```

```text
Command> Goodbye from Handmade VM
CPU halted.
```

```sh
make test-word-command-unknown
```

```text
Command> Unknown command
CPU halted.
```

## 今回まだやらないこと

今回の比較処理は、固定文字列ごとに命令列を並べている。

まだ「文字列比較サブルーチン」にはしていない。次は、同じ形の比較処理をサブルーチン風に切り出すと、プログラム全体を少し整理できる。

## line-buffer.bin の解説

`line-buffer.bin` は、単語コマンドへ進むための入口です。

前回までのプログラムは、入力を1文字ずつ見てすぐ分岐していた。今回は、すぐには分岐せず、Enterまでの文字をVM内メモリに貯める。

## 何を確認するか

確認したいことは、次の3つです。

```text
1. SYSCALL 2 で1 byte読む
2. STB [R1], R0 で入力文字をメモリへ保存する
3. 最後に0 byteを書いて、SYSCALL 1 で文字列として表示する
```

実行すると、次のようになる。

```sh
make test-line-buffer
```

```text
Type a word, then Enter
>You typed: help
CPU halted.
```

## バッファの場所

入力バッファは `0x180` から始まる。

```text
0x180: h
0x181: e
0x182: l
0x183: p
0x184: 0x00
```

この最後の `0x00` があるため、`SYSCALL 1` は `help` の終わりを判断できる。

## 読む、比べる、保存する

ループの中心は次の流れです。

```text
SYSCALL 2      1文字読む
MOVI R2, 10    LFと比べる
CMP R0, R2
JZ finish
MOVI R2, 13    CRと比べる
CMP R0, R2
JZ finish
STB [R1], R0   入力文字を保存する
INC R1         次の保存先へ進む
JUMP loop
```

`R0` は `SYSCALL 2` の結果を受け取る。`R1` はバッファ内の書き込み位置を持つ。`R2` は比較用の値として使う。

## 終端処理

Enterが来たら、Enterそのものは保存しない。

代わりに、現在の `R1` が指す場所へ `0x00` を書く。

```text
MOVI R0, 0
STB [R1], R0
```

これで、入力バッファは0終端文字列になる。

そのあと、まずラベル用の文字列を表示する。

```text
MOVI R0, 0x140
SYSCALL 1
```

続けて、入力バッファそのものを表示する。

```text
MOVI R0, 0x180
SYSCALL 1
```

ここで `R0` は文字そのものではなく、文字列の先頭アドレスとして使われる。

## 今回まだやらないこと

このサンプルは、あくまで入力バッファの入口です。

まだ `help` と `quit` の文字列比較はしない。まずは、VM内メモリに「行」として入力を置けるところまでを確認する。

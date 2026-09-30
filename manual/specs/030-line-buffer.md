## Day 30: line-buffer sample

```text
名前: line-buffer.bin
分類: sample program
目的: Enterまでの入力をVM内メモリへ1 byteずつ保存し、0終端文字列として表示する
命令長: 4 byte固定
使う命令:
  MOVI
  SYSCALL 1
  SYSCALL 2
  SYSCALL 3
  CMP
  STB
  INC
  JUMP
  JZ
  HALT
読むレジスタ: R0, R1, R2
書くレジスタ: R0, R1, R2
読むメモリ: memory[R0] から 0 byte まで
書くメモリ: memory[0x180] から最大63 byteと終端0 byte
PCの変化: fetch時に +4、JUMP/JZで即値アドレスへ変更
条件フラグの変化: CMPでzero flagを更新する
成功条件: 入力した単語をVM内メモリに保存し、Enter後に同じ文字列として表示する
```

## プログラムの形

`programs/line-buffer.bin` は、プロンプトを表示したあと、`SYSCALL 2` で1 byteずつ入力を読む。

読んだ文字が Enter ではなければ、`STB [R1], R0` で `memory[R1]` へ保存し、`INC R1` で次の保存先へ進む。

```text
0x00000000: MOVI R0, 0x100
0x00000004: SYSCALL 1
0x00000008: MOVI R1, 0x180
0x0000000C: SYSCALL 2
0x00000010: MOVI R2, 10
0x00000014: CMP R0, R2
0x00000018: JZ 0x40
...
0x00000028: STB [R1], R0
0x0000002C: INC R1
0x00000030: MOVI R2, 0x1BF
0x00000034: CMP R1, R2
0x00000038: JZ 0x40
0x0000003C: JUMP 0x0C
0x00000040: MOVI R0, 0
0x00000044: STB [R1], R0
0x00000048: MOVI R0, 0x140
0x0000004C: SYSCALL 1
0x00000050: MOVI R0, 0x180
0x00000054: SYSCALL 1
0x00000058: MOVI R0, 10
0x0000005C: SYSCALL 3
0x00000060: HALT

0x00000100: "Type a word, then Enter\n>\0"
0x00000140: "You typed: \0"
0x00000180: 入力バッファ
```

## 入力バッファ

入力バッファの先頭は `0x180`。

`R1` は「次に書く場所」を持つ。最初は `0x180` を入れておき、1文字保存するたびに `INC R1` で進める。

```text
R0: 読んだ文字
R1: 次の保存先
R2: 比較用の値
```

Enter は保存せず、代わりに `0x00` を書く。これで、入力された文字列を `SYSCALL 1` で表示できる0終端文字列として扱える。

このサンプルでは、`0x180` から `0x1BE` まで最大63 byteを入力用に使う。`R1 == 0x1BF` になった場合も、Enterと同じように終端処理へ進む。

## 実行

```sh
make test-line-buffer
```

期待出力:

```text
Type a word, then Enter
>You typed: help
CPU halted.
```

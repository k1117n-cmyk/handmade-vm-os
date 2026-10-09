## Day 32: word-command sample

```text
名前: word-command.bin
分類: sample program
目的: Enterまで入力した文字列を help / quit と比較し、複数文字コマンドとして分岐する
命令長: 4 byte固定
使う命令:
  MOVI
  SYSCALL 1
  SYSCALL 2
  CMP
  LDB
  STB
  INC
  JUMP
  JZ
  JNZ
  HALT
読むレジスタ: R0, R1, R2
書くレジスタ: R0, R1, R2
読むメモリ: memory[0x500] から0終端までの入力バッファ
書くメモリ: memory[0x500] から最大63 byteと終端0 byte
PCの変化: fetch時に +4、JUMP/JZ/JNZで即値アドレスへ変更
条件フラグの変化: CMPでzero flagを更新する
成功条件: help ならhelp文字列を表示し、quit なら終了文字列を表示し、それ以外ならunknown文字列を表示する
```

## プログラムの形

`programs/word-command.bin` は、`line-buffer.bin` と同じように1行を入力バッファへ保存する。

入力が終わったら、バッファの先頭から1 byteずつ読み、固定文字列 `help` と比較する。

```text
memory[0x500]: h
memory[0x501]: e
memory[0x502]: l
memory[0x503]: p
memory[0x504]: 0x00
```

`help` と一致しなければ、次に `quit` と比較する。

```text
memory[0x500]: q
memory[0x501]: u
memory[0x502]: i
memory[0x503]: t
memory[0x504]: 0x00
```

どちらでもなければ `Unknown command` を表示する。

## 文字列比較

VMにはまだ文字列比較命令がない。

そのため、今回のプログラムでは次の形を繰り返す。

```text
MOVI R1, 0x500
LDB R0, [R1]
MOVI R2, 'h'
CMP R0, R2
JNZ compare_quit
```

1文字でも違えば、`help` ではないと判断して `quit` の比較へ進む。

最後に `0x00` も比較する。これにより、`help` は一致するが `helpful` は一致しない。

## 実行

```sh
make test-word-command
```

期待出力:

```text
Command> Commands:
help: show help
quit: quit
CPU halted.
```

```sh
make test-word-command-quit
```

期待出力:

```text
Command> Goodbye from Handmade VM
CPU halted.
```

```sh
make test-word-command-unknown
```

期待出力:

```text
Command> Unknown command
CPU halted.
```

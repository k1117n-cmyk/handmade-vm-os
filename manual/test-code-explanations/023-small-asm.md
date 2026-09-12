## small-asm.c の解説

`tools/small-asm.c` は、`programs/hello.asm` のような小さなアセンブリファイルを、VMが読み込める `.bin` に変換するためのCプログラムです。

本格的なアセンブラではありません。最初は、外部バイナリローダーへ渡す最小プログラムを手で作りやすくするための道具です。

## 対応する命令

対応する命令は3つだけです。

```asm
MOVI Rn, imm
SYSCALL imm
HALT
```

`Rn` は `R0` から `R7` までです。

`imm` は20bitの即値として扱うので、`0` から `0xFFFFF` までです。10進数と `0x` 付きの16進数を使えます。

## 変換の流れ

入力行から、まずコメントと空白を取り除きます。

```text
MOVI R0, 65
```

カンマは空白として扱うので、次の3つに分けられます。

```text
MOVI
R0
65
```

`MOVI` の命令値は `type=4, op=0` です。

`R0` は `rd=0`、`65` は `imm=0x41` なので、命令値は次のようになります。

```text
0x40000000 | (0 << 20) | 0x41
= 0x40000041
```

## big-endianで書く

VMの `fetch` は、メモリ上の4 byteをbig-endianとして読んでいます。

そのため、`small-asm.c` も命令値をbig-endianで書きます。

```text
0x40000041 -> 40 00 00 41
0x60000000 -> 60 00 00 00
0x01000000 -> 01 00 00 00
```

## 実行方法

まず、アセンブラをコンパイルします。

```sh
cc tools/small-asm.c -o /tmp/small-asm
```

次に、`programs/hello.asm` から一時バイナリを作ります。

```sh
/tmp/small-asm programs/hello.asm /tmp/hello-small-asm.bin
```

生成したバイナリをVMで実行します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm /tmp/hello-small-asm.bin
```

期待する出力:

```text
A
CPU halted.
```

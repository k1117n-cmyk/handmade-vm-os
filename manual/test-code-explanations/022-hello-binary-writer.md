## write-hello-bin.c の解説

`tools/write-hello-bin.c` は、`programs/hello.bin` を作るための小さなCプログラムです。

本格的なアセンブラではありません。

今回は、すでに分かっている3つの命令値を、順番にファイルへ書きます。

```text
0x40000041
0x60000000
0x01000000
```

## 書き出す命令

この3つの値は、次のアセンブリに対応しています。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

`MOVI R0, 65` は、`R0` に文字 `A` の文字コードである `65` を入れます。

`SYSCALL 0` は、`R0` の下位1 byteを文字として表示します。

`HALT` はVMを停止します。

## big-endianで書く

VMの `fetch` は、メモリ上の4 byteを big-endian として読んでいます。

そのため、ファイルへ書くときも同じ順番にします。

```c
static void write_u32_be(FILE *file, uint32_t value) {
    fputc((value >> 24) & 0xFF, file);
    fputc((value >> 16) & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
    fputc(value & 0xFF, file);
}
```

例えば `0x40000041` は、次の4 byteになります。

```text
40 00 00 41
```

## 実行方法

まず、作成ツールをコンパイルします。

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
```

次に、`programs/hello.bin` を作ります。

```sh
/tmp/write-hello-bin
```

出力先を指定することもできます。

```sh
/tmp/write-hello-bin /tmp/hello.bin
```

生成したバイナリをVMで実行します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待する出力:

```text
A
CPU halted.
```

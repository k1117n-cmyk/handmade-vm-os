## 021-binary-loader-test.c の解説

`021-binary-loader-test.c` は、外部バイナリファイルをVMのメモリへ読み込めるかを確認するテストです。

このテストでは、まず `/tmp/021-program.bin` を作ります。

中身は次の3命令です。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

命令値にすると、次の12 byteになります。

```text
40 00 00 41
60 00 00 00
01 00 00 00
```

## テストの流れ

まず、テスト用のバイナリを作ります。

```c
const uint8_t program[] = {
    0x40, 0x00, 0x00, 0x41,
    0x60, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00
};
```

次に、そのファイルを `memory` の先頭へ読み込みます。

```c
fread(memory, 1, MEMORY_SIZE, file);
```

ここで大事なのは、ファイルの中身を命令として特別扱いしていないことです。

VMから見ると、外部ファイルもただのbyte列です。

```text
program.bin
  -> memory[0] から順番にコピーする
```

## 読み込み後の確認

読み込み後、メモリ先頭から4 byteずつ読みます。

```c
uint32_t inst0 = read_u32_be(memory, 0x00000000);
uint32_t inst1 = read_u32_be(memory, 0x00000004);
uint32_t inst2 = read_u32_be(memory, 0x00000008);
```

期待する値は次の通りです。

```text
inst0 = 0x40000041
inst1 = 0x60000000
inst2 = 0x01000000
```

この3つが合っていれば、外部バイナリをメモリへ読み込む最小機能は成功です。

## 実行結果

```text
inst0=0x40000041
inst1=0x60000000
inst2=0x01000000
binary loader test passed.
```

# Sample Programs

このディレクトリには、VMで読み込む小さな外部バイナリを置く。

## hello.bin

`hello.bin` は、Day 21 の外部バイナリローダー確認用の最小プログラム。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

byte列:

```text
40 00 00 41
60 00 00 00
01 00 00 00
```

再生成:

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待出力:

```text
A
CPU halted.
```

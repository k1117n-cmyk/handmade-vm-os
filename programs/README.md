# Sample Programs

このディレクトリには、VMで読み込む小さな外部バイナリを置く。

## hello.asm / hello.bin

`hello.asm` と `hello.bin` は、外部バイナリローダー確認用の最小プログラム。

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

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm programs/hello.bin
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

## echo-char.asm / echo-char.bin

`echo-char.asm` と `echo-char.bin` は、入力系 `SYSCALL 2` の最小確認用プログラム。

```asm
SYSCALL 2
SYSCALL 0
HALT
```

byte列:

```text
60 00 00 02
60 00 00 00
01 00 00 00
```

アセンブリ表記から再生成:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/echo-char.asm programs/echo-char.bin
```

実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待出力:

```text
A
CPU halted.
```

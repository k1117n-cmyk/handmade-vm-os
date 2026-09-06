## 005-decode-test.c の解説

`005-decode-test.c` は、32bitの命令値から `type`, `op`, `rd`, `imm` を取り出す decode のテストです。

ここではVMのメモリや実行ループは使っていません。1つの32bit値を用意し、その中の各bit範囲をシフトとマスクで取り出しています。

```c
uint32_t instruction = 0x40780010;
```

この `0x40780010` という命令を分解して、次の値が取り出せるかを確認します。

```text
type = 4
op   = 0
rd   = 7
imm  = 0x80010
```

## 命令フォーマット

このテストでは、32bit命令を次のような配置として扱っています。

```text
bits 31..28: type
bits 27..24: op
bits 23..20: rd
bits 19..0 : imm
```

図で見ると次の形です。

```text
[ type:4 ][ op:4 ][ rd:4 ][ imm:20 ]
```

`instruction = 0x40780010` を16進数の桁で分けると、次のようになります。

```text
0x 4 0 7 80010
   | | |   |
   | | |   +-- imm  = 0x80010
   | | +------ rd   = 7
   | +-------- op   = 0
   +---------- type = 4
```

## 上位バイトの取り出し

まず、命令の上位2バイトを取り出しています。

```c
uint8_t b0 = (instruction >> 24) & 0xFF;
uint8_t b1 = (instruction >> 16) & 0xFF;
```

`instruction` は `0x40780010` なので、バイト単位では次の並びです。

```text
0x40 0x78 0x00 0x10
```

`b0` は一番上のバイトです。

```c
b0 = (instruction >> 24) & 0xFF;
```

結果は次の通りです。

```text
b0 = 0x40
```

`b1` は上から2番目のバイトです。

```c
b1 = (instruction >> 16) & 0xFF;
```

結果は次の通りです。

```text
b1 = 0x78
```

## type の取り出し

```c
uint8_t type = (b0 >> 4) & 0x0F;
```

`b0` は `0x40` です。

```text
0x40 = 0100 0000
```

上位4bitが `type` なので、右に4bitシフトします。

```text
0100 0000 >> 4 = 0000 0100
```

したがって、

```text
type = 4
```

になります。

## op の取り出し

```c
uint8_t op = b0 & 0x0F;
```

`op` は `b0` の下位4bitです。

```text
0x40 = 0100 0000
0x0F = 0000 1111
```

`& 0x0F` で下位4bitだけを残します。

```text
0100 0000
0000 1111
---------
0000 0000
```

したがって、

```text
op = 0
```

になります。

## rd の取り出し

```c
uint8_t rd = (b1 >> 4) & 0x0F;
```

`b1` は `0x78` です。

```text
0x78 = 0111 1000
```

上位4bitが `rd` なので、右に4bitシフトします。

```text
0111 1000 >> 4 = 0000 0111
```

したがって、

```text
rd = 7
```

になります。

## imm の取り出し

```c
uint32_t imm = instruction & 0x000FFFFF;
```

`imm` は命令の下位20bitです。

`0x000FFFFF` は下位20bitだけが `1` になっているマスクです。

```text
instruction = 0x40780010
mask        = 0x000FFFFF
result      = 0x00080010
```

したがって、

```text
imm = 0x80010
```

になります。

## 表示している内容

最後に、取り出した値を `printf` で表示しています。

```c
printf("instruction=%08X\n", instruction);
printf("b0=%02X\n", b0);
printf("b1=%02X\n", b1);
printf("type=%u\n", type);
printf("op=%u\n", op);
printf("rd=%u\n", rd);
printf("imm=%05X\n", imm);
```

期待される出力は次の通りです。

```text
instruction=40780010
b0=40
b1=78
type=4
op=0
rd=7
imm=80010
```

## 整理

このコードで重要なのは、命令を実行しているのではなく、命令を分解しているだけという点です。

```text
fetch: memory から32bit命令を読む
decode: 32bit命令から type, op, rd, imm を取り出す
execute: decode した内容に従って処理を行う
```

`005-decode-test.c` は、このうち decode だけを単独で確認しています。

要するにこのコードは、`0x40780010` という32bit命令から、シフトとマスクを使って `type=4`, `op=0`, `rd=7`, `imm=0x80010` を正しく取り出せるかを見るテストです。

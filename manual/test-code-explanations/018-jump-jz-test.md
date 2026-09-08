## 018-jump-jz-test.c の解説

`018-jump-jz-test.c` は、小さな自作VM上で `JUMP` と `JZ` が正しく動くかを確認するテストです。

`JUMP` は、無条件に `PC` を指定アドレスへ変更します。

`JZ` は、`zero_flag` が `true` のときだけ `PC` を指定アドレスへ変更します。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
JUMP 0x10
MOVI R0, 99
HALT

MOVI R0, 1
MOVI R1, 1
CMP R0, R1
JZ 0x28
MOVI R2, 99
HALT

MOVI R2, 7
HALT
```

期待する結果は次の通りです。

```text
R0 = 1
R1 = 1
R2 = 7
zero_flag = true
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[MEMORY_SIZE] = {0};
uint32_t pc = 0x00000000;
uint32_t regs[8] = {0};
bool zero_flag = false;
bool running = true;
```

`pc` は、次に読む命令のアドレスです。

`zero_flag` は、直前の `CMP` の比較結果を残すフラグです。

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 68 00 00 10    JUMP 0x10
0x00000004: 40 00 00 63    MOVI R0, 99
0x00000008: 01 00 00 00    HALT
0x00000010: 40 00 00 01    MOVI R0, 1
0x00000014: 40 10 00 01    MOVI R1, 1
0x00000018: 24 01 00 00    CMP R0, R1
0x0000001C: 6A 00 00 28    JZ 0x28
0x00000020: 40 20 00 63    MOVI R2, 99
0x00000024: 01 00 00 00    HALT
0x00000028: 40 20 00 07    MOVI R2, 7
0x0000002C: 01 00 00 00    HALT
```

## 命令の読み取り

VMは1命令を4バイト固定長として読んでいます。

```c
uint32_t inst =
    ((uint32_t)memory[pc] << 24) |
    ((uint32_t)memory[pc + 1] << 16) |
    ((uint32_t)memory[pc + 2] << 8) |
    ((uint32_t)memory[pc + 3]);

pc += 4;
```

ここで `pc += 4` しているので、普通の命令なら次の命令へ進みます。

`JUMP` や `JZ` では、その後で `pc` を別の値に上書きします。

## JUMP

`JUMP 0x10` の命令は `0x68000010` です。

分解すると次のようになります。

```text
type = 6
op   = 8
imm  = 0x10
```

このVMでは、`JUMP` は `pc` を `imm` に変更します。

```c
pc = imm;
```

そのため、`0x00000004` にある `MOVI R0, 99` は実行されません。

## CMP と JZ

`CMP R0, R1` で `R0` と `R1` を比較します。

```asm
MOVI R0, 1
MOVI R1, 1
CMP R0, R1
```

どちらも `1` なので、`zero_flag` は `true` になります。

```c
zero_flag = regs[rd] == regs[rs];
```

その直後の `JZ 0x28` は、`zero_flag` が `true` のときだけジャンプします。

```c
if (zero_flag) {
    pc = imm;
}
```

今回の `zero_flag` は `true` なので、`pc` は `0x28` になります。

そのため、`0x00000020` にある `MOVI R2, 99` は実行されません。

## ジャンプ先の範囲チェック

`JUMP` と `JZ` では、ジャンプ先から4バイトの命令をfetchできるか確認します。

```c
static bool can_fetch(uint32_t address) {
    return address <= MEMORY_SIZE - 4;
}
```

ジャンプ先がメモリ範囲外なら、VMを停止します。

## 最後の判定

実行後に、レジスタと `zero_flag` を確認します。

```c
if (regs[0] == 1 && regs[1] == 1 && regs[2] == 7 && zero_flag) {
    printf("JUMP/JZ test passed.\n");
    return 0;
}
```

確認しているのは、次の2つです。

```text
1. JUMP によって途中の MOVI R0, 99 が飛ばされた
2. JZ によって途中の MOVI R2, 99 が飛ばされた
```

要するにこのコードは、`PC` を直接変更して実行順序を変える、最小の分岐命令テストです。

## 実行結果

```text
R0=0x00000001
R1=0x00000001
R2=0x00000007
zero_flag=true
JUMP/JZ test passed.
```

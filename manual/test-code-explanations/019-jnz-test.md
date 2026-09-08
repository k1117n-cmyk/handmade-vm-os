## 019-jnz-test.c の解説

`019-jnz-test.c` は、小さな自作VM上で `JNZ` が正しく動くかを確認するテストです。

`JNZ` は、`zero_flag` が `false` のときだけ `PC` を指定アドレスへ変更します。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 1
MOVI R1, 2
CMP R0, R1
JNZ 0x18
MOVI R2, 99
HALT
MOVI R2, 7
HALT
```

期待する結果は次の通りです。

```text
R0 = 1
R1 = 2
R2 = 7
zero_flag = false
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

`zero_flag` は、直前の `CMP` の比較結果を残すフラグです。

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 40 00 00 01    MOVI R0, 1
0x00000004: 40 10 00 02    MOVI R1, 2
0x00000008: 24 01 00 00    CMP R0, R1
0x0000000C: 6B 00 00 18    JNZ 0x18
0x00000010: 40 20 00 63    MOVI R2, 99
0x00000014: 01 00 00 00    HALT
0x00000018: 40 20 00 07    MOVI R2, 7
0x0000001C: 01 00 00 00    HALT
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

普通の命令なら、これで次の4バイトへ進みます。

`JNZ` では、その後で条件に応じて `pc` を別の値に上書きします。

## CMP

`CMP R0, R1` で `R0` と `R1` を比較します。

```asm
MOVI R0, 1
MOVI R1, 2
CMP R0, R1
```

`R0` は `1`、`R1` は `2` なので、`zero_flag` は `false` になります。

```c
zero_flag = regs[rd] == regs[rs];
```

## JNZ

`JNZ 0x18` の命令は `0x6B000018` です。

分解すると次のようになります。

```text
type = 6
op   = 11
imm  = 0x18
```

このVMでは、`JNZ` は `zero_flag` が `false` のときだけ `pc` を `imm` に変更します。

```c
if (!zero_flag) {
    pc = imm;
}
```

今回の `zero_flag` は `false` なので、`pc` は `0x18` になります。

そのため、`0x00000010` にある `MOVI R2, 99` は実行されません。

## ジャンプ先の範囲チェック

`JNZ` では、ジャンプ先から4バイトの命令をfetchできるか確認します。

```c
static bool can_fetch(uint32_t address) {
    return address <= MEMORY_SIZE - 4;
}
```

ジャンプ先がメモリ範囲外なら、VMを停止します。

## 最後の判定

実行後に、レジスタと `zero_flag` を確認します。

```c
if (regs[0] == 1 && regs[1] == 2 && regs[2] == 7 && !zero_flag) {
    printf("JNZ test passed.\n");
    return 0;
}
```

確認しているのは、次の2つです。

```text
1. 違う値をCMPしたことで zero_flag が false になる
2. JNZ によって途中の MOVI R2, 99 が飛ばされる
```

要するにこのコードは、`JZ` とは逆向きの条件で分岐する、最小の `JNZ` 命令テストです。

## 実行結果

```text
R0=0x00000001
R1=0x00000002
R2=0x00000007
zero_flag=false
JNZ test passed.
```

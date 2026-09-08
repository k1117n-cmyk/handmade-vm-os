## 017-cmp-test.c の解説

`017-cmp-test.c` は、小さな自作VM上で `CMP` が正しく動くかを確認するテストです。

`CMP` は、2つのレジスタの値を比較して `zero_flag` を更新します。レジスタの値そのものは変更しません。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 10
MOVI R1, 10
CMP R0, R1
MOVI R2, 10
MOVI R3, 3
CMP R2, R3
HALT
```

期待する結果は次の通りです。

```text
1回目のCMP: R0 == R1 なので zero_flag = true
2回目のCMP: R2 != R3 なので zero_flag = false
R0, R1, R2, R3 の値は変わらない
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[1024 * 1024] = {0};
uint32_t pc = 0x00000000;
uint32_t regs[8] = {0};
bool running = true;
bool zero_flag = false;
```

`zero_flag` は、直前の比較結果を残すためのフラグです。

```text
zero_flag = true:
  直前のCMPで2つの値が同じだった

zero_flag = false:
  直前のCMPで2つの値が違っていた
```

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 40 00 00 0A    MOVI R0, 10
0x00000004: 40 10 00 0A    MOVI R1, 10
0x00000008: 24 01 00 00    CMP R0, R1
0x0000000C: 40 20 00 0A    MOVI R2, 10
0x00000010: 40 30 00 03    MOVI R3, 3
0x00000014: 24 23 00 00    CMP R2, R3
0x00000018: 01 00 00 00    HALT
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

メモリ上の4バイトをビッグエンディアンとして `uint32_t` に合成しています。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint8_t rs = (inst >> 16) & 0x0F;
```

`CMP` では `rd` と `rs` を使います。

```text
[ type:4 ][ op:4 ][ rd:4 ][ rs:4 ][ unused:16 ]
```

## CMP

`CMP R0, R1` の命令は `0x24010000` です。

分解すると次のようになります。

```text
type = 2
op   = 4
rd   = 0
rs   = 1
```

このVMでは、`CMP` は `rd` と `rs` の値が同じかどうかを調べます。

```c
zero_flag = regs[rd] == regs[rs];
```

今回の1回目の比較では:

```c
zero_flag = regs[0] == regs[1];
```

`R0` も `R1` も `10` なので、`zero_flag` は `true` になります。

2回目の比較では:

```c
zero_flag = regs[2] == regs[3];
```

`R2` は `10`、`R3` は `3` なので、`zero_flag` は `false` になります。

## レジスタを書き換えない

`CMP` の大事な点は、レジスタの値を変えないことです。

```text
R0 = 10
R1 = 10
CMP R0, R1
```

を実行しても、`R0` と `R1` はどちらも `10` のままです。

`CMP` は、次のように「比較結果だけ」を残します。

```text
zero_flag = true
```

## エラーチェック

`CMP` では、実行前に `rd` と `rs` が正しいレジスタ番号か確認しています。

```c
if (rd >= 8 || rs >= 8) {
    printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
    running = false;
}
```

確認しているのは、`rd` と `rs` が `R0` から `R7` の範囲に収まっているかです。

## 最後の判定

実行後に、1回目と2回目の比較結果、そしてレジスタ値を確認します。

```c
if (first_cmp_zero && !second_cmp_zero &&
    regs[0] == 10 && regs[1] == 10 &&
    regs[2] == 10 && regs[3] == 3) {
    printf("CMP test passed.\n");
    return 0;
}
```

確認しているのは、次の2つです。

```text
1. 同じ値を比較したとき zero_flag が true になる
2. 違う値を比較したとき zero_flag が false になる
```

あわせて、`CMP` がレジスタの値を変えていないことも確認しています。

## 実行結果

```text
first_cmp_zero=true
second_cmp_zero=false
R0=0x0000000A
R1=0x0000000A
R2=0x0000000A
R3=0x00000003
CMP test passed.
```

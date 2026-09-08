## 016-add-sub-test.c の解説

`016-add-sub-test.c` は、小さな自作VM上で `ADD` と `SUB` が正しく動くかを確認するテストです。

`ADD` は、指定したレジスタに別のレジスタの値を足します。`SUB` は、指定したレジスタから別のレジスタの値を引きます。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 10
MOVI R1, 3
ADD R0, R1
MOVI R2, 10
MOVI R3, 3
SUB R2, R3
HALT
```

期待する結果は次の通りです。

```text
R0 = 0x0000000D
R2 = 0x00000007
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[1024 * 1024] = {0};
uint32_t pc = 0x00000000;
uint32_t regs[8] = {0};
bool running = true;
```

意味は次の通りです。

- `memory`: 1MBのメモリ。1要素が `uint8_t` なので1バイト単位
- `pc`: プログラムカウンタ。次に読む命令のアドレス
- `regs`: 32bitレジスタ8本、`R0` から `R7`
- `running`: VMの実行継続フラグ

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 40 00 00 0A    MOVI R0, 10
0x00000004: 40 10 00 03    MOVI R1, 3
0x00000008: 22 01 00 00    ADD R0, R1
0x0000000C: 40 20 00 0A    MOVI R2, 10
0x00000010: 40 30 00 03    MOVI R3, 3
0x00000014: 23 23 00 00    SUB R2, R3
0x00000018: 01 00 00 00    HALT
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04`, `0x08` のように4ずつ進みます。

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
uint32_t imm = inst & 0x000FFFFF;
```

`ADD` と `SUB` では `rd` と `rs` を使います。`imm` は使いません。

```text
[ type:4 ][ op:4 ][ rd:4 ][ rs:4 ][ unused:16 ]
```

## ADD

`ADD R0, R1` の命令は `0x22010000` です。

分解すると次のようになります。

```text
type = 2
op   = 2
rd   = 0
rs   = 1
```

このVMでは、`ADD` は `rd` のレジスタへ `rs` のレジスタ値を足します。

```c
regs[rd] += regs[rs];
```

今回の値を入れると:

```c
regs[0] += regs[1];
```

`R0` は `10`、`R1` は `3` なので、`ADD` 後の `R0` は `13` になります。

## SUB

`SUB R2, R3` の命令は `0x23230000` です。

分解すると次のようになります。

```text
type = 2
op   = 3
rd   = 2
rs   = 3
```

このVMでは、`SUB` は `rd` のレジスタから `rs` のレジスタ値を引きます。

```c
regs[rd] -= regs[rs];
```

今回の値を入れると:

```c
regs[2] -= regs[3];
```

`R2` は `10`、`R3` は `3` なので、`SUB` 後の `R2` は `7` になります。

## エラーチェック

`ADD` と `SUB` では、実行前に `rd` と `rs` が正しいレジスタ番号か確認しています。

```c
if (rd >= 8 || rs >= 8) {
    printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
    running = false;
}
```

確認しているのは、`rd` と `rs` が `R0` から `R7` の範囲に収まっているかです。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に `R0` と `R2` を表示し、次の条件を確認します。

```c
if (regs[0] == 0x0000000D && regs[2] == 0x00000007) {
    printf("ADD/SUB test passed.\n");
    return 0;
}
```

確認しているのは、`ADD R0, R1` によって `R0` が `10` から `13` になり、`SUB R2, R3` によって `R2` が `10` から `7` になったかどうかです。

要するにこのコードは、レジスタ同士で計算する最小の算術命令を確認する、`ADD` / `SUB` 命令のテストです。

## 実行結果

```text
R0=0x0000000D
R2=0x00000007
ADD/SUB test passed.
```

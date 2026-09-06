## 011-stdi-test.c の解説

`011-stdi-test.c` は、小さな自作VM上で `STDI`、つまり「レジスタの32bit値を、即値アドレスで指定したメモリへ4 byteで書く命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
STDI R0, 0x10      ; memory[0x10..0x13] = R0 を4 byteで書く
HALT
```

さらに、テスト用の初期値として `R0` に `0x12345678` を入れています。

big-endianで書くので、この32bit値は次の4 byteになります。

```text
12 34 56 78
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

そのあと、`STDI` で書き込む値を `R0` に入れています。

```c
regs[0] = 0x12345678;
```

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 51 00 00 10    STDI R0, 0x10
0x00000004: 01 00 00 00    HALT

0x00000010: 00 00 00 00    write destination
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04` と進みます。

`0x00000010` 以降は、`STDI` が書き込む先のメモリです。

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

最初の4バイトは、

```text
51 00 00 10
```

なので、命令値としては `0x51000010` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

`STDI` では `rs` は使いません。`rd` を読み取り元レジスタ、`imm` を書き込み先のメモリアドレスとして使います。

```text
[ type:4 ][ op:4 ][ rd:4 ][ imm:20 ]
```

## STDI

このテストで一番重要なのはここです。

```c
} else if (type == 5 && op == 1) {
    ...
    memory[imm] = (regs[rd] >> 24) & 0xFF;
    memory[imm + 1] = (regs[rd] >> 16) & 0xFF;
    memory[imm + 2] = (regs[rd] >> 8) & 0xFF;
    memory[imm + 3] = regs[rd] & 0xFF;
}
```

`STDI R0, 0x10` の命令は `0x51000010` です。

分解すると次のようになります。

```text
type = 5
op   = 1
rd   = 0
imm  = 0x10
```

このVMでは `STDI` の `rd` を「書き込む32bit値を持つレジスタ」、`imm` を「書き込み先のメモリアドレス」として使っています。

したがって、実行内容は次の通りです。

```c
memory[0x10] = (regs[0] >> 24) & 0xFF;
memory[0x11] = (regs[0] >> 16) & 0xFF;
memory[0x12] = (regs[0] >> 8) & 0xFF;
memory[0x13] = regs[0] & 0xFF;
```

この時点で、

```text
regs[0] = 0x12345678
```

なので、実際には次の4 byteが書かれます。

```text
memory[0x10] = 0x12
memory[0x11] = 0x34
memory[0x12] = 0x56
memory[0x13] = 0x78
```

## エラーチェック

`STDI` では、書き込み前に次の確認をしています。

```c
if (rd >= 8) {
    printf("invalid register: R%u\n", rd);
    running = false;
} else if (imm > sizeof(memory) - 4) {
    printf("memory address out of range: 0x%08X\n", imm);
    running = false;
}
```

確認しているのは次の2点です。

- `rd` が `R0` から `R7` の範囲に収まっているか
- `imm` から4 byte書いても `memory` の範囲内に収まるか

`STB` は1 byteだけ書きますが、`STDI` は4 byte書くので、`imm`, `imm + 1`, `imm + 2`, `imm + 3` まで範囲内である必要があります。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に `R0` と書き込み先メモリを表示し、次の条件を確認します。

```c
if (memory[0x10] == 0x12 &&
    memory[0x11] == 0x34 &&
    memory[0x12] == 0x56 &&
    memory[0x13] == 0x78) {
    printf("STDI test passed.\n");
    return 0;
}
```

確認しているのは、`R0` の32bit値が `memory[0x10..0x13]` に `12 34 56 78` として書かれたかどうかです。

要するにこのコードは、レジスタの32bit値を即値アドレスで指定したメモリ位置へbig-endianで書けるかを見る、`STDI` 命令の最小テストです。

## 実行結果

R0=0x12345678
memory[0x10..0x13]=12 34 56 78
STDI test passed.


## 010-lddi-test.c の解説

`010-lddi-test.c` は、小さな自作VM上で `LDDI`、つまり「即値アドレスで指定したメモリから4 byteを読み、32bit値としてレジスタへ入れる命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
LDDI R0, 0x10      ; R0 = memory[0x10..0x13] を32bit値として読む
HALT
```

さらに、テスト用データとして `0x10` から4 byteを置いています。

```text
12 34 56 78
```

big-endianで読むので、この4 byteは `0x12345678` になります。

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
0x00000000: 50 00 00 10    LDDI R0, 0x10
0x00000004: 01 00 00 00    HALT

0x00000010: 12
0x00000011: 34
0x00000012: 56
0x00000013: 78
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04` と進みます。

`0x00000010` 以降にある値は命令ではなく、`LDDI` が読むためのデータです。

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
50 00 00 10
```

なので、命令値としては `0x50000010` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

`LDDI` では `rs` は使いません。`imm` をメモリアドレスとして使います。

```text
[ type:4 ][ op:4 ][ rd:4 ][ imm:20 ]
```

## LDDI

このテストで一番重要なのはここです。

```c
} else if (type == 5 && op == 0) {
    ...
    regs[rd] =
        ((uint32_t)memory[imm] << 24) |
        ((uint32_t)memory[imm + 1] << 16) |
        ((uint32_t)memory[imm + 2] << 8) |
        ((uint32_t)memory[imm + 3]);
}
```

`LDDI R0, 0x10` の命令は `0x50000010` です。

分解すると次のようになります。

```text
type = 5
op   = 0
rd   = 0
imm  = 0x10
```

このVMでは `LDDI` の `rd` を「読み込んだ32bit値を書き込む先のレジスタ」、`imm` を「読み込み元のメモリアドレス」として使っています。

したがって、実行内容は次の通りです。

```c
regs[0] =
    ((uint32_t)memory[0x10] << 24) |
    ((uint32_t)memory[0x11] << 16) |
    ((uint32_t)memory[0x12] << 8) |
    ((uint32_t)memory[0x13]);
```

この時点で、

```text
memory[0x10] = 0x12
memory[0x11] = 0x34
memory[0x12] = 0x56
memory[0x13] = 0x78
```

なので、実際には次の値が作られます。

```text
0x12345678
```

つまり `R0 = 0x12345678` になります。

## エラーチェック

`LDDI` では、読み込み前に次の確認をしています。

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
- `imm` から4 byte読んでも `memory` の範囲内に収まるか

`LDB` は1 byteだけ読みますが、`LDDI` は4 byte読むので、`imm`, `imm + 1`, `imm + 2`, `imm + 3` まで範囲内である必要があります。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に `R0` を表示し、次の条件を確認します。

```c
if (regs[0] == 0x12345678) {
    printf("LDDI test passed.\n");
    return 0;
}
```

確認しているのは、`LDDI` によって `memory[0x10..0x13]` の4 byteが `0x12345678` として `R0` に入ったかどうかです。

要するにこのコードは、即値アドレスで指定したメモリ位置から4 byteをbig-endianで読み、32bit値としてレジスタへ入れられるかを見る、`LDDI` 命令の最小テストです。

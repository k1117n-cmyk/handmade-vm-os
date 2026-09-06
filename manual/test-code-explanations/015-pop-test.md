## 015-pop-test.c の解説

`015-pop-test.c` は、小さな自作VM上で `POP`、つまり「スタックから32bit値を取り出してレジスタへ入れる命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
POP R1
HALT
```

さらに、テスト用の初期状態として `SP` を `0x000FFFFC` にし、その場所に `12 34 56 78` を置いています。

期待する結果は次の通りです。

```text
R1 = 0x12345678
SP = 0x00100000
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[1024 * 1024] = {0};
uint32_t pc = 0x00000000;
uint32_t sp = 0x000FFFFC;
uint32_t regs[8] = {0};
bool running = true;
```

意味は次の通りです。

- `memory`: 1MBのメモリ。1要素が `uint8_t` なので1バイト単位
- `pc`: プログラムカウンタ。次に読む命令のアドレス
- `sp`: スタックポインタ。今のスタック先頭を指す
- `regs`: 32bitレジスタ8本、`R0` から `R7`
- `running`: VMの実行継続フラグ

## メモリに置かれるプログラム

このテストでは、VMが実行する命令とスタック上のデータを `memory` に直接書き込んでいます。

```text
0x00000000: 71 10 00 00    POP R1
0x00000004: 01 00 00 00    HALT

0x000FFFFC: 12 34 56 78    stack data
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04` と進みます。

スタックの読み取り元は、初期 `SP` が指している `0x000FFFFC` です。

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
71 10 00 00
```

なので、命令値としては `0x71100000` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
```

`POP` では `rd` を使います。`rd` はスタックから取り出した値を書き込む先のレジスタです。

```text
[ type:4 ][ op:4 ][ rd:4 ][ unused:20 ]
```

## POP

このテストで一番重要なのはここです。

```c
} else if (type == 7 && op == 1) {
    ...
    regs[rd] =
        ((uint32_t)memory[sp] << 24) |
        ((uint32_t)memory[sp + 1] << 16) |
        ((uint32_t)memory[sp + 2] << 8) |
        ((uint32_t)memory[sp + 3]);
    sp += 4;
}
```

`POP R1` の命令は `0x71100000` です。

分解すると次のようになります。

```text
type = 7
op   = 1
rd   = 1
```

このVMでは `POP` の `rd` を「スタックから取り出した32bit値を書き込む先のレジスタ」として使っています。

読み取り順は big-endian です。

```text
memory[0x000FFFFC] = 0x12
memory[0x000FFFFD] = 0x34
memory[0x000FFFFE] = 0x56
memory[0x000FFFFF] = 0x78
```

なので、`R1` には `0x12345678` が入ります。

そのあと、スタックから4 byte取り出したので、`sp` を4増やします。

```c
sp += 4;
```

`POP` 前の `sp` は `0x000FFFFC` なので、`POP` 後の `sp` は `0x00100000` になります。

## エラーチェック

`POP` では、実行前に次の確認をしています。

```c
if (rd >= 8) {
    printf("invalid register: R%u\n", rd);
    running = false;
} else if (sp > sizeof(memory) - 4) {
    printf("stack underflow\n");
    running = false;
}
```

確認しているのは次の2点です。

- `rd` が `R0` から `R7` の範囲に収まっているか
- `sp` から4 byte読んでも `memory` の範囲内に収まるか

初期状態の `SP = 0x00100000` は、スタックが空であることを表します。その状態で `POP` すると、`memory` の外を読むことになるので `stack underflow` として停止します。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に `R1` と `SP` を表示し、次の条件を確認します。

```c
if (regs[1] == 0x12345678 && sp == 0x00100000) {
    printf("POP test passed.\n");
    return 0;
}
```

確認しているのは、`POP R1` によってスタック上の4 byteが `0x12345678` として `R1` に入り、`SP` が4増えたかどうかです。

要するにこのコードは、下方向スタックから32bit値を取り出せるかを見る、`POP` 命令の最小テストです。

## 実行結果

R1=0x12345678
SP=0x00100000
POP test passed.

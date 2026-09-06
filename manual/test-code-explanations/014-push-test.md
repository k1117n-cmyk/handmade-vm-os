## 014-push-test.c の解説

`014-push-test.c` は、小さな自作VM上で `PUSH`、つまり「レジスタの32bit値をスタックへ積む命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
PUSH R0
HALT
```

さらに、テスト用の初期値として `R0` に `0x12345678` を入れています。

期待する結果は次の通りです。

```text
SP = 0x000FFFFC
memory[SP..SP+3] = 12 34 56 78
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[1024 * 1024] = {0};
uint32_t pc = 0x00000000;
uint32_t sp = 0x00100000;
uint32_t regs[8] = {0};
bool running = true;
```

意味は次の通りです。

- `memory`: 1MBのメモリ。1要素が `uint8_t` なので1バイト単位
- `pc`: プログラムカウンタ。次に読む命令のアドレス
- `sp`: スタックポインタ。次に使うスタック位置を管理する
- `regs`: 32bitレジスタ8本、`R0` から `R7`
- `running`: VMの実行継続フラグ

そのあと、`PUSH` で積む値を `R0` に入れています。

```c
regs[0] = 0x12345678;
```

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 70 00 00 00    PUSH R0
0x00000004: 01 00 00 00    HALT
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04` と進みます。

スタックの書き込み先は、初期 `SP` から4 byte下がった `0x000FFFFC` です。

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
70 00 00 00
```

なので、命令値としては `0x70000000` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
```

`PUSH` では `rd` を使います。`rd` はスタックへ積む値を持つレジスタです。

```text
[ type:4 ][ op:4 ][ rd:4 ][ unused:20 ]
```

## PUSH

このテストで一番重要なのはここです。

```c
} else if (type == 7 && op == 0) {
    ...
    sp -= 4;
    memory[sp] = (regs[rd] >> 24) & 0xFF;
    memory[sp + 1] = (regs[rd] >> 16) & 0xFF;
    memory[sp + 2] = (regs[rd] >> 8) & 0xFF;
    memory[sp + 3] = regs[rd] & 0xFF;
}
```

`PUSH R0` の命令は `0x70000000` です。

分解すると次のようになります。

```text
type = 7
op   = 0
rd   = 0
```

このVMでは `PUSH` の `rd` を「スタックへ積む32bit値を持つレジスタ」として使っています。

スタックは下方向に伸びるので、まず `sp` を4減らします。

```c
sp -= 4;
```

初期値は `0x00100000` なので、`PUSH` 後の `sp` は `0x000FFFFC` です。

その場所へ `R0` の値を big-endian で書き込みます。

```text
0x12345678
=> 12 34 56 78
```

## エラーチェック

`PUSH` では、実行前に次の確認をしています。

```c
if (rd >= 8) {
    printf("invalid register: R%u\n", rd);
    running = false;
} else if (sp < 4) {
    printf("stack overflow\n");
    running = false;
}
```

確認しているのは次の2点です。

- `rd` が `R0` から `R7` の範囲に収まっているか
- `sp` を4減らしても、書き込み先がメモリ範囲内に残るか

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に `R0`, `SP`, スタック上の4 byteを表示し、次の条件を確認します。

```c
if (sp == 0x000FFFFC &&
    memory[sp] == 0x12 &&
    memory[sp + 1] == 0x34 &&
    memory[sp + 2] == 0x56 &&
    memory[sp + 3] == 0x78) {
    printf("PUSH test passed.\n");
    return 0;
}
```

確認しているのは、`PUSH R0` によって `SP` が4減り、`R0` の32bit値がスタック上に `12 34 56 78` として書かれたかどうかです。

要するにこのコードは、下方向スタックに32bit値を積めるかを見る、`PUSH` 命令の最小テストです。

## 実行結果

R0=0x12345678
SP=0x000FFFFC
memory[SP..SP+3]=12 34 56 78
PUSH test passed.

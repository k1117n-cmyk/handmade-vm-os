## 008-ldb-test.c の解説

`008-ldb-test.c` は、小さな自作VM上で `LDB`、つまり「メモリから1バイトを読み込んでレジスタへ入れる命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R1, 0x10      ; R1 = 0x10
LDB R0, [R1]       ; R0 = memory[R1]
SYSCALL 0          ; R0 の下位8bitを文字として出力
HALT
```

さらに、命令列の直後にテスト用データとして `0x41`、つまり文字 `'A'` を置いています。

```c
memory[0x00000010] = 0x41;  // 'A'
```

つまり、`R1` にアドレス `0x10` を入れ、`LDB` で `memory[0x10]` から1バイト読み、`R0` に `0x41` が入るかを確認します。その後 `SYSCALL 0` で `R0` の下位8bitを文字として出力するので、期待される表示は `A` です。

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
0x00000000: 40 10 00 10    MOVI R1, 0x10
0x00000004: 30 01 00 00    LDB R0, [R1]
0x00000008: 60 00 00 00    SYSCALL 0
0x0000000C: 01 00 00 00    HALT
0x00000010: 41             'A'
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04`, `0x08`, `0x0C` と進みます。

最後の `0x00000010` にある `0x41` は命令ではなく、`LDB` で読み込むためのデータです。

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

たとえば、次の4バイトは、

```c
memory[0] = 0x40;
memory[1] = 0x10;
memory[2] = 0x00;
memory[3] = 0x10;
```

命令値としては `0x40100010` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint8_t rs = (inst >> 16) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

つまり32bit命令を次のように見ています。

```text
[ type:4 ][ op:4 ][ rd:4 ][ rs:4 ][ imm/unused:16 or imm:20 ]
```

このテストで使う命令は主に4種類です。

```c
type == 4 && op == 0  // MOVI
type == 3 && op == 0  // LDB
type == 6 && op == 0  // SYSCALL
inst == 0x01000000    // HALT
```

## MOVI

```c
} else if (type == 4 && op == 0) {
    regs[rd] = imm;
}
```

`MOVI` は即値 `imm` をレジスタ `rd` に入れます。

最初の命令 `0x40100010` は次のように分解されます。

```text
type = 4
op   = 0
rd   = 1
imm  = 0x10
```

したがって、実行内容は次の通りです。

```c
regs[1] = 0x10;
```

つまり `R1 = 0x10` になります。

## LDB

このテストで一番重要なのはここです。

```c
} else if (type == 3 && op == 0) {
    ...
    regs[rd] = memory[regs[rs]];
}
```

`LDB R0, [R1]` の命令は `0x30010000` です。

分解すると次のようになります。

```text
type = 3
op   = 0
rd   = 0
rs   = 1
```

このVMでは `LDB` の `rd` を「読み込んだ値を書き込む先のレジスタ」、`rs` を「読み込み元アドレスを持つレジスタ」として使っています。

したがって、実行内容は次の通りです。

```c
regs[0] = memory[regs[1]];
```

この時点で、

```c
regs[1] == 0x10
memory[0x10] == 0x41
```

なので、実際には次の読み込みが行われます。

```c
regs[0] = memory[0x10];
regs[0] = 0x41;
```

`memory` は `uint8_t` の配列なので、読み込まれるのは1バイトです。その1バイトの値が32bitレジスタ `R0` に代入されます。

## SYSCALL 0

次に `SYSCALL 0` が実行されます。

```c
} else if (type == 6 && op == 0) {
    if (imm == 0) {
        putchar(regs[0] & 0xFF);
        putchar('\n');
    } else {
        printf("unimplemented syscall: %u\n", imm);
        running = false;
    }
}
```

`SYSCALL 0` は、`R0` の下位8bitを文字として出力します。

`LDB` によって `R0` には `0x41` が入っているので、

```c
putchar(regs[0] & 0xFF);
```

は文字 `'A'` を出力します。

その直後に、

```c
putchar('\n');
```

で改行も出力します。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
    printf("CPU halted.\n");
}
```

命令が `0x01000000` なら実行ループを終了します。このテストでは停止時に `CPU halted.` も表示します。

## エラーチェック

`LDB` では、読み込み前に次の確認をしています。

```c
if (rd >= 8 || rs >= 8) {
    printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
    running = false;
} else if (regs[rs] >= sizeof(memory)) {
    printf("memory address out of range: 0x%08X\n", regs[rs]);
    running = false;
} else {
    regs[rd] = memory[regs[rs]];
}
```

確認しているのは次の2点です。

- `rd` と `rs` が `R0` から `R7` の範囲に収まっているか
- `regs[rs]` が `memory` の範囲内のアドレスか

問題がなければ、`memory[regs[rs]]` から1バイト読み、`regs[rd]` に入れます。

## 実行結果

期待される出力は次の通りです。

```text
A
CPU halted.
```

要するにこのコードは、`R1` の値をメモリアドレスとして使い、そこに置かれている1バイト `0x41` を `R0` に読み込めるかを確認する、`LDB` 命令の最小テストです。

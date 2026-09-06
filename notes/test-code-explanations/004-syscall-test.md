## 004-syscall-test.c の解説

`004-syscall-test.c` は、小さな自作VM上で `SYSCALL` 命令を確認するテストです。

この段階の `SYSCALL` は、VMの外側にある機能を呼び出すための命令です。このテストでは `SYSCALL 1` を使い、VM内メモリに置いた0終端文字列をホスト側の端末へ表示します。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 0x10      ; R0 = 0x10
SYSCALL 1          ; memory[R0] から0終端文字列を出力
HALT
```

さらに、命令列の後ろにテスト用の文字列を置いています。

```text
Hello\n\0
```

つまり、`R0` に文字列の先頭アドレス `0x10` を入れ、`SYSCALL 1` で `memory[0x10]` から順番に文字を読み、`0x00` が出るまで表示します。

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

`running` が `true` の間、VMは命令を読み続けます。`HALT` が実行されると `running = false` になり、ループが終了します。

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 40 00 00 10    MOVI R0, 0x10
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 01 00 00 00    HALT

0x00000010: 'H'
0x00000011: 'e'
0x00000012: 'l'
0x00000013: 'l'
0x00000014: 'o'
0x00000015: '\n'
0x00000016: 0x00
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04`, `0x08` と進みます。

`0x00000010` 以降にある値は命令ではなく、`SYSCALL 1` が読むための文字列データです。

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
memory[1] = 0x00;
memory[2] = 0x00;
memory[3] = 0x10;
```

命令値としては `0x40000010` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

このテストでは `rs` は使っていません。

32bit命令を大まかに次のように見ています。

```text
[ type:4 ][ op:4 ][ rd:4 ][ unused/register:4 ][ imm:20 ]
```

このテストで使う命令は主に3種類です。

```c
type == 4 && op == 0  // MOVI
type == 6 && op == 0  // SYSCALL
inst == 0x01000000    // HALT
```

## MOVI

```c
} else if (type == 4 && op == 0) {
    if (rd >= 8) {
        printf("invalid register: R%u\n", rd);
        running = false;
    } else {
        regs[rd] = imm;
    }
}
```

`MOVI` は即値 `imm` をレジスタ `rd` に入れます。

最初の命令 `0x40000010` は次のように分解されます。

```text
type = 4
op   = 0
rd   = 0
imm  = 0x10
```

したがって、実行内容は次の通りです。

```c
regs[0] = 0x10;
```

つまり `R0 = 0x10` になります。

この `R0` は、次の `SYSCALL 1` で文字列の先頭アドレスとして使われます。

## SYSCALL 1

このテストで一番重要なのはここです。

```c
} else if (type == 6 && op == 0) {
    if (imm == 1) {
        uint32_t address = regs[0];
        while (address < sizeof(memory) && memory[address] != 0) {
            putchar(memory[address]);
            address++;
        }
    } else {
        printf("unimplemented syscall: %u\n", imm);
        running = false;
    }
}
```

`SYSCALL 1` の命令は `0x60000001` です。

分解すると次のようになります。

```text
type = 6
op   = 0
imm  = 1
```

`imm` が syscall 番号として使われています。`imm == 1` なので、文字列表示の処理に入ります。

```c
uint32_t address = regs[0];
```

ここで `R0` の値を読みます。直前の `MOVI` によって `R0 = 0x10` なので、

```c
address = 0x10;
```

になります。

その後、次のループでメモリから1バイトずつ読み、文字として出力します。

```c
while (address < sizeof(memory) && memory[address] != 0) {
    putchar(memory[address]);
    address++;
}
```

ループの意味は次の通りです。

- `address < sizeof(memory)`: メモリ範囲外を読まないための確認
- `memory[address] != 0`: 0終端に到達していないかの確認
- `putchar(memory[address])`: 現在の1バイトを文字として出力
- `address++`: 次の文字へ進む

このテストでは `memory[0x10]` から `memory[0x15]` までが文字列で、`memory[0x16]` が `0x00` です。

したがって、`SYSCALL 1` は次の文字列を表示します。

```text
Hello
```

文字列内に `'\n'` が含まれているので、`Hello` の後で改行されます。

## 未実装syscall

`SYSCALL` 命令で `imm` が `1` 以外だった場合は、未実装として扱われます。

```c
printf("unimplemented syscall: %u\n", imm);
running = false;
```

このテストでは `imm == 1` の命令だけを使っているので、この分岐には入りません。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
    printf("CPU halted.\n");
}
```

命令が `0x01000000` なら実行ループを終了します。このテストでは停止時に `CPU halted.` も表示します。

## 実行結果

期待される出力は次の通りです。

```text
Hello
CPU halted.
```

実際には文字列の最後に `'\n'` が入っているため、`Hello` の表示後に改行され、その次の行に `CPU halted.` が表示されます。

## 整理

このコードで重要なのは、`SYSCALL` 命令の番号を `imm` から取り出し、その番号に応じてVM外側の処理を呼び出している点です。

```text
SYSCALL 1:
  R0を文字列の先頭アドレスとして使う
  memory[R0] から 0x00 まで1バイトずつ読む
  読んだバイトをhost端末へ出力する
```

要するにこのコードは、`R0` の値をメモリアドレスとして使い、そこから `0x00` が出るまで1バイトずつ文字を表示できるかを確認する、`SYSCALL 1` の最小テストです。

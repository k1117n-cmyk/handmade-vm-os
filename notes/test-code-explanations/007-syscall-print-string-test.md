## 007-syscall-print-string-test.c の解説

`007-syscall-print-string-test.c` は、小さな自作VM上で `SYSCALL 1`、つまり「VM内メモリに置かれた0終端文字列を表示する処理」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 0x10      ; R0 = 0x10
SYSCALL 1          ; memory[R0] から0終端文字列を出力
HALT
```

さらに、命令列の後ろにテスト用の文字列を置いています。

```text
Hello from Handmade OS\n\0
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
0x00000015: ' '
0x00000016: 'f'
0x00000017: 'r'
0x00000018: 'o'
0x00000019: 'm'
0x0000001A: ' '
0x0000001B: 'H'
0x0000001C: 'a'
0x0000001D: 'n'
0x0000001E: 'd'
0x0000001F: 'm'
0x00000020: 'a'
0x00000021: 'd'
0x00000022: 'e'
0x00000023: ' '
0x00000024: 'O'
0x00000025: 'S'
0x00000026: '\n'
0x00000027: 0x00
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
    if (imm == 0) {
        putchar(regs[0] & 0xFF);
        putchar('\n');
    } else if (imm == 1) {
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

このテストでは `memory[0x10]` から `memory[0x26]` までが文字列で、`memory[0x27]` が `0x00` です。

したがって、`SYSCALL 1` は次の文字列を表示します。

```text
Hello from Handmade OS
```

文字列内に `'\n'` が含まれているので、`OS` の後で改行されます。

## SYSCALL 0 との違い

このコードには `SYSCALL 0` の処理も残っています。

```c
if (imm == 0) {
    putchar(regs[0] & 0xFF);
    putchar('\n');
}
```

`SYSCALL 0` は `R0` の下位8bitを1文字として出力します。

一方、今回使っている `SYSCALL 1` は、`R0` を文字そのものではなく「文字列の先頭アドレス」として使います。

```text
SYSCALL 0: R0 の値を1文字として出す
SYSCALL 1: R0 をアドレスとして使い、memory上の0終端文字列を出す
```

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
Hello from Handmade OS
CPU halted.
```

実際には文字列の最後に `'\n'` が入っているため、`Hello from Handmade OS` の表示後に改行され、その次の行に `CPU halted.` が表示されます。

## 整理

このコードで重要なのは、`pc` と `R0` が別の役割を持っていることです。

```text
pc: 次に実行する命令の場所を指す
R0: SYSCALL 1 が読む文字列データの場所を指す
```

要するにこのコードは、`R0` の値をメモリアドレスとして使い、そこから `0x00` が出るまで1バイトずつ文字を表示できるかを確認する、`SYSCALL 1` の最小テストです。

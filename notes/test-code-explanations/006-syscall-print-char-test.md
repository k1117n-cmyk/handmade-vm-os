## 006-syscall-print-char-test.c の解説

`006-syscall-print-char-test.c` は、小さな自作VM上で `SYSCALL 0`、つまり「`R0` の下位8bitを1文字として表示する処理」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R0, 65        ; R0 = 65 = 0x41 = 'A'
SYSCALL 0          ; R0 の下位8bitを文字として出力
HALT
```

つまり、`R0` に `65`、16進数では `0x41` を入れ、`SYSCALL 0` でそれを文字として表示します。ASCIIでは `0x41` が `'A'` なので、期待される表示は `A` です。

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
0x00000000: 40 00 00 41    MOVI R0, 65
0x00000004: 60 00 00 00    SYSCALL 0
0x00000008: 01 00 00 00    HALT
```

命令は4バイト固定長なので、`pc` は `0x00`, `0x04`, `0x08` と進みます。

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
memory[3] = 0x41;
```

命令値としては `0x40000041` になります。

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
    regs[rd] = imm;
}
```

`MOVI` は即値 `imm` をレジスタ `rd` に入れます。

最初の命令 `0x40000041` は次のように分解されます。

```text
type = 4
op   = 0
rd   = 0
imm  = 0x41
```

したがって、実行内容は次の通りです。

```c
regs[0] = 0x41;
```

つまり `R0 = 65` になります。

この `R0` は、次の `SYSCALL 0` で出力する文字の値として使われます。

## SYSCALL 0

このテストで一番重要なのはここです。

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

`SYSCALL 0` の命令は `0x60000000` です。

分解すると次のようになります。

```text
type = 6
op   = 0
imm  = 0
```

`imm` が syscall 番号として使われています。`imm == 0` なので、1文字表示の処理に入ります。

```c
putchar(regs[0] & 0xFF);
```

ここで `R0` の下位8bitを取り出し、文字として出力します。

この時点で、

```c
regs[0] == 0x41
```

なので、実際には次のような意味になります。

```c
putchar(0x41);
```

ASCIIでは `0x41` は `'A'` なので、画面には `A` が表示されます。

その直後に、

```c
putchar('\n');
```

で改行も出力します。

`& 0xFF` しているのは、`R0` は32bitレジスタですが、`SYSCALL 0` ではそのうち下位1バイトだけを文字として扱うためです。

## 未実装syscall

`SYSCALL` 命令で `imm` が `0` 以外だった場合は、未実装として扱われます。

```c
printf("unimplemented syscall: %u\n", imm);
running = false;
```

このテストでは `imm == 0` の命令だけを使っているので、この分岐には入りません。

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
A
CPU halted.
```

実行の流れを整理すると、次のようになります。

```text
1. MOVI R0, 65
   R0 = 0x41

2. SYSCALL 0
   R0 の下位8bitを文字として出力
   0x41 = 'A'

3. HALT
   CPU halted.
```

要するにこのコードは、`R0` に入っている値の下位1バイトを文字として表示できるかを確認する、`SYSCALL 0` の最小テストです。

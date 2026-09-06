# Instruction Fields

このメモは、VMの命令を decode したあとに出てくる `type`, `op`, `rd`, `rs`, `imm` が、アセンブリ表記のどの部分に対応するかを確認するための一覧表。

命令タイプと命令一覧だけを探す場合は、先に [../instruction-types.md](../instruction-types.md) を見る。このファイルは、fieldをCコードの実行にどう結びつけるかを理解するための補助資料。

## 32bit命令の基本形

このVMの命令は、基本的に4 byte、つまり32bit。

```text
bits 31..28  bits 27..24  bits 23..20  bits 19..16  bits 15..0
+----------+------------+------------+------------+----------------+
|   type   |     op     |     rd     |     rs     |     unused     |
+----------+------------+------------+------------+----------------+
```

即値を使う命令では、後ろ20bitを `imm` として使う。

```text
bits 31..28  bits 27..24  bits 23..20  bits 19..0
+----------+------------+------------+-------------------------+
|   type   |     op     |     rd     |           imm           |
+----------+------------+------------+-------------------------+
```

## Field一覧

| field | 読み方 | 意味 | アセンブリでの見え方 | Cでの見え方 |
|---|---|---|---|---|
| `type` | タイプ | 命令の大分類 | `MOVI`系、`LDB`系、`SYSCALL`系など | `if (type == 3)` |
| `op` | オペレーション | 同じ分類の中の命令番号 | `LDB`、`STB`、`MOVI`などの具体的な命令 | `if (op == 0)` |
| `rd` | アールディー | destination register。主に書き込み先レジスタ | `MOVI R1, 0x10` の `R1` | `regs[rd] = ...` |
| `rs` | アールエス | source register。主に読み取り元レジスタ | `LDB R0, [R1]` の `R1` | `regs[rs]` |
| `imm` | イミディエイト | 命令の中に直接入っている数値 | `MOVI R1, 0x10` の `0x10` | `regs[rd] = imm` |

## fieldとCでの実行の読み方

`field` と `Cでの実行` は、アセンブリを書く人の目線ではなく、VMを作る人の目線。

アセンブリを書く側なら、まずはこれだけでよい。

```asm
MOVI R1, 0x10
```

意味:

```text
R1に0x10を入れる
```

VMを作る側は、さらにこう考える。

```text
この4 byteの命令は、どの部分が R1 を表しているのか？
この4 byteの命令は、どの部分が 0x10 を表しているのか？
```

この「命令の中の部品」が `field`。

例:

```text
MOVI R1, 0x10
```

は、memory上ではこう置かれる。

```text
40 10 00 10
```

1つの32bit命令として見ると:

```text
0x40100010
```

これを区切ると:

```text
0x 4 0 1 00010
   | | |   |
   | | |   +-- imm = 0x10
   | | +------ rd = 1
   | +-------- op = 0
   +---------- type = 4
```

つまり:

```text
MOVI R1, 0x10
```

の中身は、VMから見ると:

```text
type = 4
op   = 0
rd   = 1
imm  = 0x10
```

になる。

`Cでの実行` は、その命令をVM内部でどう処理するか。

アセンブリでは:

```asm
MOVI R1, 0x10
```

Cでは、レジスタを配列で持っているので:

```c
regs[1] = 0x10;
```

になる。

ただしVMは、毎回 `R1` 専用のコードを書くわけではない。

```c
regs[1] = 0x10;
```

ではなく、decodeして取り出した値を使って、汎用的に書く。

```c
regs[rd] = imm;
```

今回だけに当てはめると:

```text
rd = 1
imm = 0x10
```

なので:

```c
regs[rd] = imm;
```

は:

```c
regs[1] = 0x10;
```

と同じ意味になる。

対応表:

| アセンブリ | field | Cでの実行 |
|---|---|---|
| `MOVI R1, 0x10` | `rd = 1` | `regs[rd] = imm` |
|  | `imm = 0x10` | `regs[1] = 0x10` |

覚え方:

```text
field = 命令を分解して取り出した部品
Cでの実行 = その部品を使ってVMがやる処理
```

`MOVI R1, 0x10` の場合:

```text
rd  = R1の番号
imm = 0x10
regs[rd] = imm
```

つまり:

```text
R1に0x10を入れる
```

## MOV

アセンブリ:

```asm
MOV R2, R0
```

意味:

```text
R0の値をR2へコピーする。
R0の値はそのまま残る。
```

field:

```text
type = 1
op   = 0
rd   = 2
rs   = 0
```

命令コード:

```text
0x10200000
  1 0 2 0 0 0 0 0
```

memory配置:

```text
0x00000004: 10 20 00 00    MOV R2, R0
```

Cでの実行:

```c
regs[rd] = regs[rs];
```

今回の値を入れると:

```c
regs[2] = regs[0];
```

## Register番号

今のVMでは、通常レジスタは `R0` から `R7` まで。

| アセンブリ | Cの配列 | 番号 |
|---|---|---|
| `R0` | `regs[0]` | `0` |
| `R1` | `regs[1]` | `1` |
| `R2` | `regs[2]` | `2` |
| `R3` | `regs[3]` | `3` |
| `R4` | `regs[4]` | `4` |
| `R5` | `regs[5]` | `5` |
| `R6` | `regs[6]` | `6` |
| `R7` | `regs[7]` | `7` |

そのため、`rd >= 8` や `rs >= 8` は不正なレジスタ番号。

```c
if (rd >= 8 || rs >= 8) {
    printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
}
```

## MOVI

アセンブリ:

```asm
MOVI R1, 0x10
```

意味:

```text
R1に0x10を入れる
```

field:

```text
type = 4
op   = 0
rd   = 1
imm  = 0x10
```

命令コード:

```text
0x40100010
  4 0 1 0 0 0 1 0
```

memory配置:

```text
0x00000000: 40 10 00 10    MOVI R1, 0x10
```

Cでの実行:

```c
regs[rd] = imm;
```

今回の値を入れると:

```c
regs[1] = 0x10;
```

## INC / DEC

アセンブリ:

```asm
INC R0
DEC R1
```

意味:

```text
INC R0 は、R0の値を1増やす。
DEC R1 は、R1の値を1減らす。
```

field:

```text
INC:
type = 2
op   = 0
rd   = 0

DEC:
type = 2
op   = 1
rd   = 1
```

命令コード:

```text
INC R0: 0x20000000
DEC R1: 0x21100000
```

memory配置:

```text
0x00000008: 20 00 00 00    INC R0
0x0000000C: 21 10 00 00    DEC R1
```

Cでの実行:

```c
regs[rd] += 1;
regs[rd] -= 1;
```

今回の値を入れると:

```c
regs[0] += 1;
regs[1] -= 1;
```

## LDB

アセンブリ:

```asm
LDB R0, [R1]
```

意味:

```text
R1の中身をアドレスとして使う。
そのmemoryから1 byte読んでR0へ入れる。
```

field:

```text
type = 3
op   = 0
rd   = 0
rs   = 1
```

命令コード:

```text
0x30010000
  3 0 0 1 0 0 0 0
```

memory配置:

```text
0x00000004: 30 01 00 00    LDB R0, [R1]
```

Cでの実行:

```c
regs[rd] = memory[regs[rs]];
```

今回の値を順番に入れると:

```c
regs[0] = memory[regs[1]];
regs[0] = memory[0x10];
regs[0] = 0x41;
```

対応表:

| アセンブリ | C | 意味 |
|---|---|---|
| `R0` | `regs[0]` | 書き込み先レジスタ |
| `R1` | `regs[1]` | アドレスを持つレジスタ |
| `[R1]` | `memory[regs[1]]` | R1が指すmemory |
| `LDB R0, [R1]` | `regs[0] = memory[regs[1]]` | memoryから1 byte読んでR0へ入れる |

## SYSCALL

アセンブリ:

```asm
SYSCALL 0
```

意味:

```text
VMの外側の機能を呼ぶ。
SYSCALL 0 は、今のVMでは R0 の下位8bitを1文字として表示する。
```

field:

```text
type = 6
op   = 0
imm  = 0
```

命令コード:

```text
0x60000000
  6 0 0 0 0 0 0 0
```

memory配置:

```text
0x00000008: 60 00 00 00    SYSCALL 0
```

Cでの実行:

```c
putchar(regs[0] & 0xFF);
putchar('\n');
```

## LDDI

アセンブリ:

```asm
LDDI R0, 0x10
```

意味:

```text
0x10を直接memory addressとして使う。
memory[0x10]から4 byte読んで、32bit値としてR0へ入れる。
```

field:

```text
type = 5
op   = 0
rd   = 0
imm  = 0x10
```

命令コード:

```text
0x50000010
  5 0 0 0 0 0 1 0
```

memory配置:

```text
0x00000000: 50 00 00 10    LDDI R0, 0x10
0x00000010: 12 34 56 78    data
```

Cでの実行:

```c
regs[rd] =
    ((uint32_t)memory[imm] << 24) |
    ((uint32_t)memory[imm + 1] << 16) |
    ((uint32_t)memory[imm + 2] << 8) |
    ((uint32_t)memory[imm + 3]);
```

今回の値を入れると:

```c
regs[0] = 0x12345678;
```

## STDI

アセンブリ:

```asm
STDI R0, 0x10
```

意味:

```text
0x10を直接memory addressとして使う。
R0の32bit値をmemory[0x10]から4 byteで書く。
```

field:

```text
type = 5
op   = 1
rd   = 0
imm  = 0x10
```

命令コード:

```text
0x51000010
  5 1 0 0 0 0 1 0
```

memory配置:

```text
0x00000000: 51 00 00 10    STDI R0, 0x10
0x00000010: 12 34 56 78    write result
```

Cでの実行:

```c
memory[imm] = (regs[rd] >> 24) & 0xFF;
memory[imm + 1] = (regs[rd] >> 16) & 0xFF;
memory[imm + 2] = (regs[rd] >> 8) & 0xFF;
memory[imm + 3] = regs[rd] & 0xFF;
```

今回の値を入れると:

```c
memory[0x10] = 0x12;
memory[0x11] = 0x34;
memory[0x12] = 0x56;
memory[0x13] = 0x78;
```

## PUSH

アセンブリ:

```asm
PUSH R0
```

意味:

```text
SPを4減らす。
R0の32bit値をmemory[SP]から4 byteで書く。
```

field:

```text
type = 7
op   = 0
rd   = 0
```

命令コード:

```text
0x70000000
  7 0 0 0 0 0 0 0
```

memory配置:

```text
0x00000000: 70 00 00 00    PUSH R0
0x000FFFFC: 12 34 56 78    stack data after PUSH
```

Cでの実行:

```c
sp -= 4;
memory[sp] = (regs[rd] >> 24) & 0xFF;
memory[sp + 1] = (regs[rd] >> 16) & 0xFF;
memory[sp + 2] = (regs[rd] >> 8) & 0xFF;
memory[sp + 3] = regs[rd] & 0xFF;
```

## POP

アセンブリ:

```asm
POP R1
```

意味:

```text
memory[SP]から4 byteを読む。
読んだ32bit値をR1へ入れる。
SPを4増やす。
```

field:

```text
type = 7
op   = 1
rd   = 1
```

命令コード:

```text
0x71100000
  7 1 1 0 0 0 0 0
```

memory配置:

```text
0x00000000: 71 10 00 00    POP R1
0x000FFFFC: 12 34 56 78    stack data before POP
```

Cでの実行:

```c
regs[rd] =
    ((uint32_t)memory[sp] << 24) |
    ((uint32_t)memory[sp + 1] << 16) |
    ((uint32_t)memory[sp + 2] << 8) |
    ((uint32_t)memory[sp + 3]);
sp += 4;
```

## HALT

アセンブリ:

```asm
HALT
```

意味:

```text
VMを止める
```

命令コード:

```text
0x01000000
  0 1 0 0 0 0 0 0
```

memory配置:

```text
0x0000000C: 01 00 00 00    HALT
```

Cでの実行:

```c
running = false;
printf("CPU halted.\n");
```

## Day 8の全体対応

アセンブリ:

```asm
MOVI R1, 0x10
LDB R0, [R1]
SYSCALL 0
HALT
```

memory:

```text
0x00000000: 40 10 00 10    MOVI R1, 0x10
0x00000004: 30 01 00 00    LDB R0, [R1]
0x00000008: 60 00 00 00    SYSCALL 0
0x0000000C: 01 00 00 00    HALT

0x00000010: 41             'A'
```

実行結果:

```text
MOVI R1, 0x10
=> regs[1] = 0x10

LDB R0, [R1]
=> regs[0] = memory[regs[1]]
=> regs[0] = memory[0x10]
=> regs[0] = 0x41

SYSCALL 0
=> R0の下位8bitを文字として表示
=> A

HALT
=> CPU halted.
```

## 覚え方

```text
rd = 書き込み先
rs = 読み取り元
imm = 命令の中に直接入っている数字
[] = レジスタの中身をアドレスとして使い、memoryを見る
```

特に `LDB R0, [R1]` は:

```text
R0  = 入れ先
R1  = アドレスを持っている
[R1] = R1が指すmemory
```

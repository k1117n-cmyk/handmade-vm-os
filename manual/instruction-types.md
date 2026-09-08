# VM Instruction Types

このファイルは、このVM専用の命令タイプと命令一覧を見るための索引です。

詳しい説明は、次のファイルに分けています。

```text
specs/                    命令ごとの仕様
reference/                fieldや命令分類の読み方
test-code-explanations/   テストコードの解説
```

## 命令フォーマット

このVMの命令は基本的に4 byte、つまり32bitです。

レジスタ同士、またはレジスタとメモリを使う命令では、次の形を使います。

```text
bits 31..28  bits 27..24  bits 23..20  bits 19..16  bits 15..0
+----------+------------+------------+------------+----------------+
|   type   |     op     |     rd     |     rs     |     unused     |
+----------+------------+------------+------------+----------------+
```

即値を使う命令では、下位20bitを `imm` として使います。

```text
bits 31..28  bits 27..24  bits 23..20  bits 19..0
+----------+------------+------------+-------------------------+
|   type   |     op     |     rd     |           imm           |
+----------+------------+------------+-------------------------+
```

## Field一覧

| field | 意味 |
|---|---|
| `type` | 命令の大分類 |
| `op` | 同じ分類の中の命令番号 |
| `rd` | destination register。主に書き込み先レジスタ |
| `rs` | source register。主に読み取り元レジスタ |
| `imm` | 命令の中に直接入っている数値 |

fieldの詳しい読み方は [reference/instruction-fields.md](reference/instruction-fields.md) を参照します。

## Type一覧

| type | 分類 | 役割 | 現在の命令 |
|---:|---|---|---|
| `1` | register instruction | レジスタ同士で値を動かす | `MOV` |
| `2` | arithmetic-register instruction | レジスタの値を計算・比較する | `INC`, `DEC`, `ADD`, `SUB`, `CMP` |
| `3` | memory-register instruction | レジスタが指すメモリを読み書きする | `LDB`, `STB` |
| `4` | immediate instruction | 命令内の即値を使う | `MOVI` |
| `5` | direct memory instruction | 命令内の即値アドレスでメモリを読み書きする | `LDDI`, `STDI` |
| `6` | syscall / control flow instruction | VM外側のサービス呼び出し、またはPC変更 | `SYSCALL 0`, `SYSCALL 1`, `JUMP`, `CALLI`, `JZ`, `JNZ` |
| `7` | stack instruction | スタックへ値を積む、または取り出す | `PUSH`, `POP` |

未使用の `type` は、現時点では未定義です。

`HALT` と `RET` は例外です。現在のVMでは `type` / `op` の組み合わせではなく、命令全体で判定しています。

```text
HALT: inst == 0x01000000
RET:  inst == 0x02000000
```

## 命令一覧

| 命令 | 識別 | 主なfield | 分類 | 仕様 |
|---|---|---|---|---|
| `HALT` | `inst == 0x01000000` | なし | system instruction | [specs/001-halt.md](specs/001-halt.md) |
| `RET` | `inst == 0x02000000` | `SP` | system / control flow instruction | [specs/020-calli-ret.md](specs/020-calli-ret.md) |
| `MOV rd, rs` | `type=1, op=0` | `rd`, `rs` | register instruction | [specs/013-mov.md](specs/013-mov.md) |
| `INC rd` | `type=2, op=0` | `rd` | arithmetic-register instruction | [specs/012-inc-dec.md](specs/012-inc-dec.md) |
| `DEC rd` | `type=2, op=1` | `rd` | arithmetic-register instruction | [specs/012-inc-dec.md](specs/012-inc-dec.md) |
| `ADD rd, rs` | `type=2, op=2` | `rd`, `rs` | arithmetic-register instruction | [specs/016-add-sub.md](specs/016-add-sub.md) |
| `SUB rd, rs` | `type=2, op=3` | `rd`, `rs` | arithmetic-register instruction | [specs/016-add-sub.md](specs/016-add-sub.md) |
| `CMP rd, rs` | `type=2, op=4` | `rd`, `rs` | arithmetic-register instruction | [specs/017-cmp.md](specs/017-cmp.md) |
| `MOVI rd, imm` | `type=4, op=0` | `rd`, `imm` | immediate instruction | [specs/003-movi.md](specs/003-movi.md) |
| `SYSCALL imm` | `type=6, op=0` | `imm` | syscall instruction | [specs/004-syscall.md](specs/004-syscall.md) |
| `SYSCALL 0` | `type=6, op=0, imm=0` | `imm`, `R0` | syscall instruction | [specs/006-syscall-print-char.md](specs/006-syscall-print-char.md) |
| `SYSCALL 1` | `type=6, op=0, imm=1` | `imm`, `R0` | syscall instruction | [specs/007-syscall-print-string.md](specs/007-syscall-print-string.md) |
| `JUMP imm` | `type=6, op=8` | `imm` | control flow instruction | [specs/018-jump-jz.md](specs/018-jump-jz.md) |
| `CALLI imm` | `type=6, op=9` | `imm`, `SP` | control flow instruction | [specs/020-calli-ret.md](specs/020-calli-ret.md) |
| `JZ imm` | `type=6, op=10` | `imm`, `zero_flag` | control flow instruction | [specs/018-jump-jz.md](specs/018-jump-jz.md) |
| `JNZ imm` | `type=6, op=11` | `imm`, `zero_flag` | control flow instruction | [specs/019-jnz.md](specs/019-jnz.md) |
| `LDB rd, [rs]` | `type=3, op=0` | `rd`, `rs` | memory-register instruction | [specs/008-ldb.md](specs/008-ldb.md) |
| `STB [rd], rs` | `type=3, op=1` | `rd`, `rs` | memory-register instruction | [specs/009-stb.md](specs/009-stb.md) |
| `LDDI rd, imm` | `type=5, op=0` | `rd`, `imm` | direct memory instruction | [specs/010-lddi.md](specs/010-lddi.md) |
| `STDI rd, imm` | `type=5, op=1` | `rd`, `imm` | direct memory instruction | [specs/011-stdi.md](specs/011-stdi.md) |
| `PUSH rd` | `type=7, op=0` | `rd`, `SP` | stack instruction | [specs/014-push.md](specs/014-push.md) |
| `POP rd` | `type=7, op=1` | `rd`, `SP` | stack instruction | [specs/015-pop.md](specs/015-pop.md) |

## 命令値の例

| アセンブリ | 命令値 | byte列 |
|---|---|---|
| `HALT` | `0x01000000` | `01 00 00 00` |
| `RET` | `0x02000000` | `02 00 00 00` |
| `MOV R2, R0` | `0x10200000` | `10 20 00 00` |
| `INC R0` | `0x20000000` | `20 00 00 00` |
| `DEC R1` | `0x21100000` | `21 10 00 00` |
| `ADD R0, R1` | `0x22010000` | `22 01 00 00` |
| `SUB R2, R3` | `0x23230000` | `23 23 00 00` |
| `CMP R0, R1` | `0x24010000` | `24 01 00 00` |
| `MOVI R1, 0x10` | `0x40100010` | `40 10 00 10` |
| `SYSCALL 0` | `0x60000000` | `60 00 00 00` |
| `SYSCALL 1` | `0x60000001` | `60 00 00 01` |
| `JUMP 0x10` | `0x68000010` | `68 00 00 10` |
| `CALLI 0x20` | `0x69000020` | `69 00 00 20` |
| `JZ 0x20` | `0x6A000020` | `6A 00 00 20` |
| `JNZ 0x20` | `0x6B000020` | `6B 00 00 20` |
| `LDB R0, [R1]` | `0x30010000` | `30 01 00 00` |
| `STB [R1], R0` | `0x31100000` | `31 10 00 00` |
| `LDDI R0, 0x10` | `0x50000010` | `50 00 00 10` |
| `STDI R0, 0x10` | `0x51000010` | `51 00 00 10` |
| `PUSH R0` | `0x70000000` | `70 00 00 00` |
| `POP R1` | `0x71100000` | `71 10 00 00` |

## Decodeの流れ

基本のdecodeは次の形です。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint8_t rs = (inst >> 16) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

命令選択は、まず `HALT` を特別扱いし、その後に `type` と `op` を見ます。

```c
if (inst == 0x01000000) {
    // HALT
} else if (inst == 0x02000000) {
    // RET
} else if (type == 1 && op == 0) {
    // MOV
} else if (type == 2 && op == 0) {
    // INC
} else if (type == 2 && op == 1) {
    // DEC
} else if (type == 2 && op == 2) {
    // ADD
} else if (type == 2 && op == 3) {
    // SUB
} else if (type == 2 && op == 4) {
    // CMP
} else if (type == 3 && op == 0) {
    // LDB
} else if (type == 3 && op == 1) {
    // STB
} else if (type == 4 && op == 0) {
    // MOVI
} else if (type == 5 && op == 0) {
    // LDDI
} else if (type == 5 && op == 1) {
    // STDI
} else if (type == 6 && op == 0) {
    // SYSCALL
} else if (type == 6 && op == 8) {
    // JUMP
} else if (type == 6 && op == 9) {
    // CALLI
} else if (type == 6 && op == 10) {
    // JZ
} else if (type == 6 && op == 11) {
    // JNZ
} else if (type == 7 && op == 0) {
    // PUSH
} else if (type == 7 && op == 1) {
    // POP
}
```

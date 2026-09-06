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
| `3` | memory-register instruction | レジスタが指すメモリを読み書きする | `LDB`, `STB` |
| `4` | immediate instruction | 命令内の即値を使う | `MOVI` |
| `6` | syscall instruction | VM外側のサービスを呼ぶ | `SYSCALL 0`, `SYSCALL 1` |

未使用の `type` は、現時点では未定義です。

`HALT` は例外です。現在のVMでは `type` / `op` の組み合わせではなく、命令全体が `0x01000000` か、上位1 byteの `opcode` が `0x01` かで判定しています。

## 命令一覧

| 命令 | 識別 | 主なfield | 分類 | 仕様 |
|---|---|---|---|---|
| `HALT` | `inst == 0x01000000` | なし | system instruction | [specs/001-halt.md](specs/001-halt.md) |
| `MOVI rd, imm` | `type=4, op=0` | `rd`, `imm` | immediate instruction | [specs/003-movi.md](specs/003-movi.md) |
| `SYSCALL imm` | `type=6, op=0` | `imm` | syscall instruction | [specs/004-syscall.md](specs/004-syscall.md) |
| `SYSCALL 0` | `type=6, op=0, imm=0` | `imm`, `R0` | syscall instruction | [specs/006-syscall-print-char.md](specs/006-syscall-print-char.md) |
| `SYSCALL 1` | `type=6, op=0, imm=1` | `imm`, `R0` | syscall instruction | [specs/007-syscall-print-string.md](specs/007-syscall-print-string.md) |
| `LDB rd, [rs]` | `type=3, op=0` | `rd`, `rs` | memory-register instruction | [specs/008-ldb.md](specs/008-ldb.md) |
| `STB [rd], rs` | `type=3, op=1` | `rd`, `rs` | memory-register instruction | [specs/009-stb.md](specs/009-stb.md) |

## 命令値の例

| アセンブリ | 命令値 | byte列 |
|---|---|---|
| `HALT` | `0x01000000` | `01 00 00 00` |
| `MOVI R1, 0x10` | `0x40100010` | `40 10 00 10` |
| `SYSCALL 0` | `0x60000000` | `60 00 00 00` |
| `SYSCALL 1` | `0x60000001` | `60 00 00 01` |
| `LDB R0, [R1]` | `0x30010000` | `30 01 00 00` |
| `STB [R1], R0` | `0x31100000` | `31 10 00 00` |

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
} else if (type == 3 && op == 0) {
    // LDB
} else if (type == 3 && op == 1) {
    // STB
} else if (type == 4 && op == 0) {
    // MOVI
} else if (type == 6 && op == 0) {
    // SYSCALL
}
```

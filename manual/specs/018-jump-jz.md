## Day 18: JUMP / JZ

```text
名前: JUMP
分類: control flow instruction
目的: PCを即値アドレスへ変更し、次に実行する命令位置を変える
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 8
  bits 23..20: unused = 0
  bits 19..0 : imm = jump target address
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4 した後、PC = imm
条件フラグの変化: なし
エラー時: imm が命令fetchできない範囲なら停止
手作りテスト: JUMP 0x10 -> 0x68000010
成功条件: JUMP直後の命令を飛ばして、0x10 の命令を実行する
```

```text
名前: JZ
分類: control flow instruction
目的: zero_flag が true のときだけ PCを即値アドレスへ変更する
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 10
  bits 23..20: unused = 0
  bits 19..0 : imm = jump target address
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4。zero_flag == true なら PC = imm
条件フラグの変化: なし
エラー時: imm が命令fetchできない範囲なら停止
手作りテスト: JZ 0x20 -> 0x6A000020
成功条件: CMPでzero_flagがtrueになった後、JZで0x20へ移動する
```

`JUMP` と `JZ` は、プログラムの流れを変える命令です。

これまでのVMは、基本的に `PC` が4ずつ増え、メモリに置いた命令を上から順番に実行していました。

```text
0x00 の命令
0x04 の命令
0x08 の命令
0x0C の命令
```

`JUMP` は、この流れを強制的に変えます。

```text
JUMP 0x10
```

なら、次に実行する命令位置を `0x10` にします。

```c
vm->pc = inst.imm;
```

`JZ` は、`zero_flag` が `true` のときだけジャンプします。

```c
if (vm->zero_flag) {
    vm->pc = inst.imm;
}
```

`zero_flag` は Day 17 の `CMP` で更新します。

```asm
MOVI R0, 5
MOVI R1, 5
CMP R0, R1
JZ same
```

この場合、`R0` と `R1` は同じなので `zero_flag` が `true` になり、`JZ` はジャンプします。

## 名前について

ブログでは、読みやすさを優先して `JUMP` / `JZ` と呼ぶ。

lesson 04 の教材側では、同じ immediate address を使うジャンプ命令を `JPI` / `JPZI` と呼んでいる。

```text
ブログ上の名前    教材寄りの名前
JUMP              JPI
JZ                JPZI
```

このリポジトリでは、Day 18時点では `JUMP` / `JZ` として説明し、命令番号は教材側に寄せて `type=6, op=8` と `type=6, op=10` を使う。

テストプログラム:

```text
JUMP 0x10
MOVI R0, 99
HALT
MOVI R0, 1
MOVI R1, 1
CMP R0, R1
JZ 0x28
MOVI R2, 99
HALT
MOVI R2, 7
HALT
```

メモリ配置:

```text
0x00000000: 68 00 00 10    JUMP 0x10
0x00000004: 40 00 00 63    MOVI R0, 99
0x00000008: 01 00 00 00    HALT
0x00000010: 40 00 00 01    MOVI R0, 1
0x00000014: 40 10 00 01    MOVI R1, 1
0x00000018: 24 01 00 00    CMP R0, R1
0x0000001C: 6A 00 00 28    JZ 0x28
0x00000020: 40 20 00 63    MOVI R2, 99
0x00000024: 01 00 00 00    HALT
0x00000028: 40 20 00 07    MOVI R2, 7
0x0000002C: 01 00 00 00    HALT
```

成功条件:

```text
R0 = 1
R1 = 1
R2 = 7
zero_flag = true
```

もし `JUMP` が動かなければ、`MOVI R0, 99` が実行される。

もし `JZ` が動かなければ、`MOVI R2, 99` が実行される。

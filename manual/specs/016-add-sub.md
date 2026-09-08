## Day 16: ADD / SUB

```text
名前: ADD
分類: arithmetic-register instruction
目的: destination register の32bit値に source register の32bit値を足す
命令長: 4 byte
bit配置:
  bits 31..28: type = 2
  bits 27..24: op = 2
  bits 23..20: rd = destination register
  bits 19..16: rs = source register
  bits 15..0 : unused = 0
読むレジスタ: rd, rs
書くレジスタ: rd
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外なら停止
手作りテスト: ADD R0, R1 -> 0x22010000
成功条件: R0 = 10, R1 = 3 のとき、実行後 R0 = 13
```

```text
名前: SUB
分類: arithmetic-register instruction
目的: destination register の32bit値から source register の32bit値を引く
命令長: 4 byte
bit配置:
  bits 31..28: type = 2
  bits 27..24: op = 3
  bits 23..20: rd = destination register
  bits 19..16: rs = source register
  bits 15..0 : unused = 0
読むレジスタ: rd, rs
書くレジスタ: rd
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外なら停止
手作りテスト: SUB R2, R3 -> 0x23230000
成功条件: R2 = 10, R3 = 3 のとき、実行後 R2 = 7
```

`ADD` と `SUB` は、レジスタ同士で計算する命令です。

```text
ADD R0, R1
```

は、次の意味になります。

```c
regs[0] = regs[0] + regs[1];
```

```text
SUB R2, R3
```

は、次の意味になります。

```c
regs[2] = regs[2] - regs[3];
```

`regs` は `uint32_t` なので、計算は32bit unsigned値として行います。

```text
0xFFFFFFFF + 1 = 0x00000000
0x00000000 - 1 = 0xFFFFFFFF
```

ただし、Day 16の最小テストでは wraparound ではなく、普通の足し算と引き算を確認します。

テストプログラム:

```text
MOVI R0, 10
MOVI R1, 3
ADD R0, R1
MOVI R2, 10
MOVI R3, 3
SUB R2, R3
HALT
```

メモリ配置:

```text
0x00000000: 40 00 00 0A    MOVI R0, 10
0x00000004: 40 10 00 03    MOVI R1, 3
0x00000008: 22 01 00 00    ADD R0, R1
0x0000000C: 40 20 00 0A    MOVI R2, 10
0x00000010: 40 30 00 03    MOVI R3, 3
0x00000014: 23 23 00 00    SUB R2, R3
0x00000018: 01 00 00 00    HALT
```

```text
ADD R0, R1

0x 2 2 0 1 0000
   | | | |   |
   | | | |   +-- unused = 0
   | | | +------ rs = 1
   | | +-------- rd = 0
   | +---------- op = 2
   +------------ type = 2
```

```text
SUB R2, R3

0x 2 3 2 3 0000
   | | | |   |
   | | | |   +-- unused = 0
   | | | +------ rs = 3
   | | +-------- rd = 2
   | +---------- op = 3
   +------------ type = 2
```

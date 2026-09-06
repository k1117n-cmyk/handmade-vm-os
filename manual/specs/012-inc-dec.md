## Day 12: INC / DEC

```text
名前: INC
分類: arithmetic-register instruction
目的: レジスタの32bit値に1を足す
命令長: 4 byte
bit配置:
  bits 31..28: type = 2
  bits 27..24: op = 0
  bits 23..20: rd = target register
  bits 19..0 : unused = 0
読むレジスタ: rd
書くレジスタ: rd
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外なら停止
手作りテスト: INC R0 -> 0x20000000
成功条件: R0 = 0x10 のとき、実行後 R0 = 0x11
```

```text
名前: DEC
分類: arithmetic-register instruction
目的: レジスタの32bit値から1を引く
命令長: 4 byte
bit配置:
  bits 31..28: type = 2
  bits 27..24: op = 1
  bits 23..20: rd = target register
  bits 19..0 : unused = 0
読むレジスタ: rd
書くレジスタ: rd
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外なら停止
手作りテスト: DEC R1 -> 0x21100000
成功条件: R1 = 0x10 のとき、実行後 R1 = 0x0F
```

`regs` は `uint32_t` なので、増減は32bit unsigned値として行います。

```text
0xFFFFFFFF + 1 = 0x00000000
0x00000000 - 1 = 0xFFFFFFFF
```

ただし、Day 12の最小テストでは wraparound ではなく、普通の `+1` と `-1` を確認します。

テストプログラム:

```text
MOVI R0, 0x10
MOVI R1, 0x10
INC R0
DEC R1
HALT
```

メモリ配置:

```text
0x00000000: 40 00 00 10    MOVI R0, 0x10
0x00000004: 40 10 00 10    MOVI R1, 0x10
0x00000008: 20 00 00 00    INC R0
0x0000000C: 21 10 00 00    DEC R1
0x00000010: 01 00 00 00    HALT
```

Cで書くなら:

```c
regs[rd] += 1;
regs[rd] -= 1;
```

```text
INC R0

0x 2 0 0 00000
   | | |   |
   | | |   +-- unused = 0
   | | +------ rd = 0
   | +-------- op = 0
   +---------- type = 2
```

```text
DEC R1

0x 2 1 1 00000
   | | |   |
   | | |   +-- unused = 0
   | | +------ rd = 1
   | +-------- op = 1
   +---------- type = 2
```

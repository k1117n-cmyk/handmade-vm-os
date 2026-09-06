## Day 10: LDDI

```text
名前: LDDI
分類: direct memory instruction
目的: VM内メモリから4 byteを読み、32bit値としてレジスタへ入れる
命令長: 4 byte
bit配置:
  bits 31..28: type = 5
  bits 27..24: op = 0
  bits 23..20: rd = destination register
  bits 19..0 : imm = immediate address
読むレジスタ: なし
書くレジスタ: rd
読むメモリ: memory[imm] から4 byte
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外、または address..address+3 が memory 範囲外なら停止
手作りテスト: LDDI R0, 0x10 -> 0x50000010
成功条件: memory[0x10..0x13] = 12 34 56 78 のとき、実行後 R0 = 0x12345678
```

テストプログラム:

```text
LDDI R0, 0x10
HALT
```

メモリ配置:

```text
0x00000000: 50 00 00 10    LDDI R0, 0x10
0x00000004: 01 00 00 00    HALT

0x00000010: 12 34 56 78    data
```

重要なのは、4 byteをどの順番で32bit値にするかです。

big-endianで読むので、次の4 byteは、

```text
12 34 56 78
```

次の32bit値になります。

```text
0x12345678
```

Cで書くなら:

```c
regs[rd] =
    ((uint32_t)memory[imm] << 24) |
    ((uint32_t)memory[imm + 1] << 16) |
    ((uint32_t)memory[imm + 2] << 8) |
    ((uint32_t)memory[imm + 3]);
```

```text
0x 5 0 0 00010
   | | |   |
   | | |   +-- imm = 0x10
   | | +------ rd = 0
   | +-------- op = 0
   +---------- type = 5
```

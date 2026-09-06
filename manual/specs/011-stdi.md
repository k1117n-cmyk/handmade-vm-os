## Day 11: STDI

```text
名前: STDI
分類: direct memory instruction
目的: レジスタの32bit値を、即値アドレスで指定したVM内メモリへ4 byteで書く
命令長: 4 byte
bit配置:
  bits 31..28: type = 5
  bits 27..24: op = 1
  bits 23..20: rd = source register
  bits 19..0 : imm = immediate address
読むレジスタ: rd
書くレジスタ: なし
読むメモリ: なし
書くメモリ: memory[imm] から memory[imm + 3] までの4 byte
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外、または address..address+3 が memory 範囲外なら停止
手作りテスト: STDI R0, 0x10 -> 0x51000010
成功条件: R0 = 0x12345678 のとき、実行後 memory[0x10..0x13] = 12 34 56 78
```

テストプログラム:

```text
STDI R0, 0x10
HALT
```

メモリ配置:

```text
0x00000000: 51 00 00 10    STDI R0, 0x10
0x00000004: 01 00 00 00    HALT

0x00000010: 00 00 00 00    write destination
```

重要なのは、32bit値をどの順番で4 byteに分けるかです。

big-endianで書くので、次の32bit値は、

```text
0x12345678
```

次の4 byteになります。

```text
12 34 56 78
```

Cで書くなら:

```c
memory[imm] = (regs[rd] >> 24) & 0xFF;
memory[imm + 1] = (regs[rd] >> 16) & 0xFF;
memory[imm + 2] = (regs[rd] >> 8) & 0xFF;
memory[imm + 3] = regs[rd] & 0xFF;
```

```text
0x 5 1 0 00010
   | | |   |
   | | |   +-- imm = 0x10
   | | +------ rd = 0
   | +-------- op = 1
   +---------- type = 5
```

## Day 14: PUSH

```text
名前: PUSH
分類: stack instruction
目的: レジスタの32bit値をスタックへ積む
命令長: 4 byte
bit配置:
  bits 31..28: type = 7
  bits 27..24: op = 0
  bits 23..20: rd = source register
  bits 19..0 : unused = 0
読むレジスタ: rd, SP
書くレジスタ: SP
読むメモリ: なし
書くメモリ: memory[SP - 4] から memory[SP - 1] までの4 byte
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: register number が範囲外、または SP - 4 から4 byte書けないなら停止
手作りテスト: PUSH R0 -> 0x70000000
成功条件: R0 = 0x12345678, SP = 0x00100000 のとき、実行後 SP = 0x000FFFFC かつ memory[SP..SP+3] = 12 34 56 78
```

テストプログラム:

```text
PUSH R0
HALT
```

メモリ配置:

```text
0x00000000: 70 00 00 00    PUSH R0
0x00000004: 01 00 00 00    HALT

0x000FFFFC: 00 00 00 00    stack write destination
```

このVMのスタックは下方向に伸びます。

```text
初期SP: 0x00100000
PUSH後: 0x000FFFFC
```

`PUSH` は、まず `SP` を4減らしてから、その場所へ32bit値を書きます。

```text
sp -= 4;
memory[sp..sp+3] = regs[rd];
```

書き込み順は `STDI` と同じく big-endian です。

```text
0x12345678
=> 12 34 56 78
```

Cで書くなら:

```c
sp -= 4;
memory[sp] = (regs[rd] >> 24) & 0xFF;
memory[sp + 1] = (regs[rd] >> 16) & 0xFF;
memory[sp + 2] = (regs[rd] >> 8) & 0xFF;
memory[sp + 3] = regs[rd] & 0xFF;
```

```text
0x 7 0 0 00000
   | | |   |
   | | |   +-- unused = 0
   | | +------ rd = 0
   | +-------- op = 0
   +---------- type = 7
```

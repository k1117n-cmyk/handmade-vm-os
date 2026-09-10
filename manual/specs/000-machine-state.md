### Day 1: Machine State

- memory size: 1MB
- address size: 32bit 
- register count: 8
- register names: R0, R1, R2, R3, R4, R5, R6, R7
- PC initial value: 0x00000000
- valid memory range: 0x00000000 - 0x000FFFFF
- SP initial value: 0x00100000 
- stack direction: downward
- zero_flag initial value: false

現在の学習VMでは、条件分岐用のフラグとしてまず `zero_flag` だけを持つ。

```text
zero_flag = true:
  直前のCMPで2つの値が同じだった

zero_flag = false:
  直前のCMPで2つの値が違っていた
```

`underflow_flag`, `overflow_flag`, `CR` レジスタは、自作OS側で必要になった時点で追加する。

```text
  31    28 27    24 23    20 19                               0
+---------+---------+---------+---------------------------------+
|  type   |   op    |   rd    |               imm               |
+---------+---------+---------+---------------------------------+
  (4bit)    (4bit)    (4bit)               (20bit)
```

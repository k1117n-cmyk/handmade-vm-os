### Day 1: Machine State

- memory size: 1MB
- address size: 32bit 
- register count: 8
- register names: R0, R1, R2, R3, R4, R5, R6, R7
- PC initial value: 0x00000000
- valid memory range: 0x00000000 - 0x000FFFFF
- SP initial value: 0x00100000 
- stack direction: downward

```text
  31    28 27    24 23    20 19                               0
+---------+---------+---------+---------------------------------+
|  type   |   op    |   rd    |               imm               |
+---------+---------+---------+---------------------------------+
  (4bit)    (4bit)    (4bit)               (20bit)
```

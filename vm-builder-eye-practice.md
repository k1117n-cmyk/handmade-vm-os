# VM Builder Eye Practice

このメモは、「アセンブリを書く人の目線」から「VMを作る人の目線」へ切り替える練習用ガイド。

VMを作る人の目線は、かなりの部分が慣れで身につく。ただし、ただ量をこなすより、毎回同じ形で分解すると早い。

## 3つの目線

命令を見るときは、毎回この3つに分ける。

```text
1. アセンブリを書く人の目線
2. 命令をdecodeするVMの目線
3. 実際に値を動かすCコードの目線
```

## MOVIの練習

命令:

```asm
MOVI R1, 0x10
```

アセンブリ目線:

```text
R1に0x10を入れる
```

decode目線:

```text
type = 4
op   = 0
rd   = 1
imm  = 0x10
```

C目線:

```c
regs[rd] = imm;
```

今回の値で置き換える:

```c
regs[1] = 0x10;
```

## LDBの練習

命令:

```asm
LDB R0, [R1]
```

アセンブリ目線:

```text
R1が指すmemoryから1 byte読んでR0へ入れる
```

decode目線:

```text
type = 3
op   = 0
rd   = 0
rs   = 1
```

C目線:

```c
regs[rd] = memory[regs[rs]];
```

今回の値で置き換える:

```c
regs[0] = memory[regs[1]];
```

もし先に:

```text
regs[1] = 0x10
memory[0x10] = 0x41
```

が分かっているなら、さらに置き換える。

```c
regs[0] = memory[0x10];
regs[0] = 0x41;
```

## いきなりCを読まない

最初からこのCコードを読もうとすると難しい。

```c
regs[rd] = memory[regs[rs]];
```

先にアセンブリを見る。

```asm
LDB R0, [R1]
```

次に decode の結果を書く。

```text
rd = 0
rs = 1
```

それからCに置き換える。

```c
regs[0] = memory[regs[1]];
```

さらに実際の値に置き換える。

```text
regs[1] = 0x10
```

だから:

```c
regs[0] = memory[0x10];
```

この「置き換え」がVM目線の練習。

## 毎回の練習フォーム

新しい命令を見たら、この形でノートに書く。

```text
命令:

アセンブリ目線:

decode目線:
type =
op   =
rd   =
rs   =
imm  =

C目線:

今回の値で置き換える:
```

使わないfieldは空欄でよい。

例えば `MOVI` は `rs` を使わない。

```text
命令:
MOVI R1, 0x10

アセンブリ目線:
R1に0x10を入れる

decode目線:
type = 4
op   = 0
rd   = 1
rs   =
imm  = 0x10

C目線:
regs[rd] = imm;

今回の値で置き換える:
regs[1] = 0x10;
```

## 覚え方

```text
アセンブリ目線 = 何をしたいか
decode目線     = 命令を部品に分ける
C目線          = その部品でVMの状態を変える
```

慣れている人は、この変換を頭の中で一瞬でやっている。

最初は紙に書いてよい。むしろ紙に書くほうが正しい。

## Day 8でやる練習

次の2つを、3回ずつ手で追う。

```asm
MOVI R1, 0x10
LDB R0, [R1]
```

1回目はゆっくり書く。

2回目は `rd`, `rs`, `imm` を意識して書く。

3回目はCの置き換えまで書く。

目標は、次の対応が自然に見えること。

```text
R0              = regs[0]
R1              = regs[1]
[R1]            = memory[regs[1]]
LDB R0, [R1]    = regs[0] = memory[regs[1]]
```

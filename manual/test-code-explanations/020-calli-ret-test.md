## 020-calli-ret-test.c の解説

`020-calli-ret-test.c` は、小さな自作VM上で `CALLI` と `RET` が正しく動くかを確認するテストです。

`CALLI` は、現在の `PC` をスタックへ積んでから、指定アドレスへジャンプします。

`RET` は、スタックから戻り先 `PC` を取り出して、その場所へ戻ります。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
CALLI 0x20
MOVI R1, 7
HALT

MOVI R0, 42
RET
```

期待する結果は次の通りです。

```text
R0 = 42
R1 = 7
SP = 0x00100000
```

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[MEMORY_SIZE] = {0};
uint32_t pc = 0x00000000;
uint32_t sp = 0x00100000;
uint32_t regs[8] = {0};
bool running = true;
```

`pc` は、次に読む命令のアドレスです。

`sp` はスタックポインタです。このVMのスタックは下方向に伸びます。

```text
PUSH/CALLI前: SP = 0x00100000
PUSH/CALLI後: SP = 0x000FFFFC
POP/RET後:    SP = 0x00100000
```

## メモリに置かれるプログラム

このテストでは、VMが実行する命令を `memory` に直接書き込んでいます。

```text
0x00000000: 69 00 00 20    CALLI 0x20
0x00000004: 40 10 00 07    MOVI R1, 7
0x00000008: 01 00 00 00    HALT
0x00000020: 40 00 00 2A    MOVI R0, 42
0x00000024: 02 00 00 00    RET
```

## 命令の読み取り

VMは1命令を4バイト固定長として読んでいます。

```c
uint32_t inst = read_u32_be(memory, pc);

pc += 4;
```

ここで `pc += 4` しているので、`CALLI` 実行時の `pc` はすでに次の命令を指しています。

```text
CALLI 0x20 をfetchした直後:
PC = 0x00000004
```

この `0x00000004` が戻り先アドレスになります。

## CALLI

`CALLI 0x20` の命令は `0x69000020` です。

分解すると次のようになります。

```text
type = 6
op   = 9
imm  = 0x20
```

このVMでは、`CALLI` は戻り先PCをスタックへ積んでから、`pc` を `imm` に変更します。

```c
sp -= 4;
write_u32_be(memory, sp, pc);
pc = imm;
```

今回の場合、戻り先は `0x00000004` です。

```text
SP = 0x000FFFFC
memory[0x000FFFFC..0x000FFFFF] = 00 00 00 04
PC = 0x00000020
```

## RET

`RET` の命令は `0x02000000` です。

`RET` は、スタックから戻り先PCを読みます。

```c
uint32_t return_address = read_u32_be(memory, sp);
sp += 4;
pc = return_address;
```

今回の場合、スタックには `0x00000004` が入っています。

そのため、`RET` 後は `CALLI` の次の命令へ戻ります。

```text
PC = 0x00000004
SP = 0x00100000
```

その後、`MOVI R1, 7` が実行されます。

## 最後の判定

実行後に、レジスタと `SP` を確認します。

```c
if (regs[0] == 42 && regs[1] == 7 && sp == 0x00100000) {
    printf("CALLI/RET test passed.\n");
    return 0;
}
```

確認しているのは、次の3つです。

```text
1. CALLI で 0x20 に移動し、MOVI R0, 42 が実行された
2. RET で CALLI の次へ戻り、MOVI R1, 7 が実行された
3. CALLIで積んだ戻り先PCがRETで取り出され、SPが元に戻った
```

要するにこのコードは、スタックを使って「呼ぶ、戻る」を確認する最小の `CALLI` / `RET` 命令テストです。

## 実行結果

```text
R0=0x0000002A
R1=0x00000007
SP=0x00100000
CALLI/RET test passed.
```

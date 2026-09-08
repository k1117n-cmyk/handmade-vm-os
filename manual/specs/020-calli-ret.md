## Day 20: CALLI / RET

```text
名前: CALLI
分類: control flow instruction
目的: 現在のPCをスタックへ積み、PCを即値アドレスへ変更する
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 9
  bits 23..20: unused = 0
  bits 19..0 : imm = call target address
読むレジスタ: SP
書くレジスタ: SP
読むメモリ: なし
書くメモリ: memory[SP - 4] から memory[SP - 1] までの4 byte
PCの変化: fetch時に +4。そのPCをスタックへ積み、PC = imm
条件フラグの変化: なし
エラー時: SP - 4 から4 byte書けない、または imm が命令fetchできない範囲なら停止
手作りテスト: CALLI 0x20 -> 0x69000020
成功条件: 戻り先PCがstackへ積まれ、0x20 の命令を実行する
```

```text
名前: RET
分類: system instruction / control flow instruction
目的: スタックから戻り先PCを取り出し、PCへ入れる
命令長: 4 byte
bit配置:
  instruction = 0x02000000
読むレジスタ: SP
書くレジスタ: SP
読むメモリ: memory[SP] から memory[SP + 3] までの4 byte
書くメモリ: なし
PCの変化: fetch時に +4 した後、stackから読んだ値をPCへ入れる
条件フラグの変化: なし
エラー時: SP から4 byte読めない、または戻り先PCが命令fetchできない範囲なら停止
手作りテスト: RET -> 0x02000000
成功条件: CALLI直後の次の命令へ戻る
```

`CALLI` と `RET` は、サブルーチンを作るための命令です。

`JUMP` は、指定した場所へ移動するだけでした。

```asm
JUMP 0x20
```

`CALLI` は、移動する前に「戻り先」をスタックへ保存します。

```asm
CALLI 0x20
```

この命令では、fetch後の `PC` が戻り先になります。

```text
0x00000000: CALLI 0x20
0x00000004: MOVI R1, 7
```

`CALLI` をfetchすると、先に `PC` は `0x00000004` になります。

この `0x00000004` をスタックへ積んでから、`PC` を `0x20` に変更します。

```c
vm->sp -= 4;
write_u32_be(vm->memory, vm->sp, vm->pc);
vm->pc = inst.imm;
```

`RET` はその逆です。

```c
uint32_t return_address = read_u32_be(vm->memory, vm->sp);
vm->sp += 4;
vm->pc = return_address;
```

これで、呼び出し先の処理が終わったあと、`CALLI` の次の命令へ戻れます。

テストプログラム:

```text
CALLI 0x20
MOVI R1, 7
HALT

0x20:
MOVI R0, 42
RET
```

メモリ配置:

```text
0x00000000: 69 00 00 20    CALLI 0x20
0x00000004: 40 10 00 07    MOVI R1, 7
0x00000008: 01 00 00 00    HALT
0x00000020: 40 00 00 2A    MOVI R0, 42
0x00000024: 02 00 00 00    RET
```

成功条件:

```text
R0 = 42
R1 = 7
SP = 0x00100000
```

もし `CALLI` が動かなければ、`R0` は `42` にならない。

もし `RET` が動かなければ、`CALLI` の次にある `MOVI R1, 7` へ戻れない。

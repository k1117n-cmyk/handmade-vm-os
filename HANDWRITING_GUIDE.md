# Handmade VM Handwriting Guide

このガイドは、自作CPU、自作OS、自作シェル、自作エディターを、学習しながら一から手書きで組み上げるための進め方をまとめたもの。

目的は、完成コードを早く得ることではなく、各層の仕様を自分の言葉で理解し、自分の手で実装できる状態にすること。

最初の命令 `HALT` のやさしい説明は `HALT_FOR_BEGINNERS.md` に分けてある。

`HALT` を実際に手書きで実装する作業手順は `WORKBOOK_001_HALT.md` にまとめる。

  今後は、いきなり完成形に飛ばさずに、

  命令形式
  ↓
  手計算
  ↓
  memory配置
  ↓
  fetch
  ↓
  decode
  ↓
  execute
  ↓
  確認

  の順で、1段ずつ確認しながら進めます。コードを書く場合も、まず「なぜその値になるか」を説明してから進めます。

## Core Rule

コードを書く前に、必ず小さな仕様を書く。

最初から大きな仕様書を書く必要はない。1命令、1関数、1syscall だけを対象にしてよい。

基本サイクル:

```text
1. 今日扱う対象を1つ決める
2. 仕様カードを書く
3. 手で入力例と期待結果を作る
4. 最小コードを書く
5. 実行して観察する
6. ログと結果をノートに残す
7. 必要なら既存コードと照合する
```

## What Not To Do

避けたい進め方:

- 既存の `lc3.c` や `emu.py` を丸写しする
- OS全体を読んでから実装しようとする
- まだ使わない命令をまとめて実装する
- アセンブラ、VM、OS、シェル、エディターを同時に進める
- 動いた理由を確認せずに次へ進む

最初は、遅くてもよいので「自分で説明できる単位」だけを進める。

## Project Notes

学習用ノートを作る。

推奨構成:

```text
handmade-vm/
  notes/
    001-machine-state.md
    002-fetch.md
    003-halt.md
    004-movi.md
    005-syscall.md
    006-string.md
  src/
    vm.c
  asm/
    os.asm
  tools/
    asm.py
```

ノートはきれいに書く必要はない。重要なのは、後で自分が「なぜそう実装したか」を追えること。

## Spec Card Template

命令や syscall を実装する前に、次の形でカードを書く。

```text
名前:
分類:
目的:
命令長:
bit配置:
読むレジスタ:
書くレジスタ:
読むメモリ:
書くメモリ:
PCの変化:
条件フラグの変化:
エラー時:
手作りテスト:
成功条件:
```

全部を完璧に埋めなくてよい。分からない項目は `未定` と書き、実装中に埋める。

## Instruction Cards

### HALT

```text
名前: HALT
分類: system instruction
目的: VMを停止する
命令長: 4 byte
bit配置: 0x01000000
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: fetch時にPCから4 byte
書くメモリ: なし
PCの変化: fetch時に +4。その後停止
条件フラグの変化: なし
エラー時: なし
手作りテスト: memory[0x00000000..0x00000003] = 01 00 00 00
成功条件: CPU halted と表示して終了
```

### MOVI

```text
名前: MOVI
分類: immediate instruction
目的: レジスタに20bit即値を入れる
命令長: 4 byte
bit配置:
  bits 31..28: type = 4
  bits 27..24: op = 0
  bits 23..20: destination register
  bits 19..0 : immediate
読むレジスタ: なし
書くレジスタ: destination register
読むメモリ: fetch時にPCから4 byte
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: 最初は変えない。必要なら後で設計する
エラー時: register number が範囲外なら停止
手作りテスト: MOVI R7, 0x80010 -> 0x40780010
成功条件: 実行後 R7 = 0x80010
```

### SYSCALL

```text
名前: SYSCALL
分類: control immediate instruction
目的: VM外側のサービスを呼ぶ
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number
読むレジスタ: syscallごとに決める。最初はR0
書くレジスタ: syscallごとに決める
読むメモリ: syscallごとに決める
書くメモリ: syscallごとに決める
PCの変化: fetch時に +4
条件フラグの変化: 最初は変えない
エラー時: 未実装syscallなら番号を表示して停止
手作りテスト: SYSCALL 1 -> 0x60000001
成功条件: R0が指す0終端文字列を表示する
```

### LDB

```text
名前: LDB
分類: memory-register instruction
目的: VM内メモリから1 byteを読み、レジスタへ入れる
命令長: 4 byte
bit配置:
  bits 31..28: type = 3
  bits 27..24: op = 0
  bits 23..20: destination register
  bits 19..16: address register
  bits 15..0 : unused = 0
読むレジスタ: address register
書くレジスタ: destination register
読むメモリ: memory[address register] から1 byte
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: 最初は変えない。必要なら後で設計する
エラー時: register number が範囲外、または address が memory 範囲外なら停止
手作りテスト: LDB R0, [R1] -> 0x30010000
成功条件: R1 = 0x10, memory[0x10] = 0x41 のとき、実行後 R0 = 0x41
```

## First Week Plan

### Day 1: Machine State

目的:

```text
VMが持つ状態を決める
```

決めるもの:

- memory size
- address size
- register count
- register names
- PC initial value
- SP initial value

確認:

```text
起動時に PC と SP を表示する
PC=00000000
SP=100000
```

### Day 2: Fetch

目的:

```text
PCが指す4 byteを1命令として読む
```

確認すること:

- byte-addressed memory
- big-endian instruction
- fetch後にPCを4増やす

手作り入力:

```text
address 0x00000000:
01 00 00 00
```

期待:

```text
fetched instruction = 0x01000000
PC = 0x00000004
```

### Day 3: HALT

目的:

```text
最初の命令を実行してVMを止める
```

成功条件:

```text
CPU halted.
```

### Day 4: Decode

目的:

```text
32bit命令から type, op, rd, imm を取り出す
```

例:

```text
instruction = 0x40780010
b0 = 0x40
b1 = 0x78
type = 4
op = 0
rd = 7
imm = 0x80010
```

### Day 5: MOVI

目的:

```text
レジスタに即値を入れる
```
```text
b0 = 0x40
b1 = 0x71
type = 4
op = 0
rd = 7
imm = 0x12345

  16進数1桁は4bitです。だから並べると、

  0x40712345
    4 0 7 1 2 3 4 5

  それでメモリにはこう入れます。

  memory[0x00000000] = 0x40;
  memory[0x00000001] = 0x71;
  memory[0x00000002] = 0x23;
  memory[0x00000003] = 0x45;

    つまり対応関係はこうです。

  memory[0] = 0x40  // type=4, op=0
  memory[1] = 0x71  // rd=7, immediate上位4bit=1
  memory[2] = 0x23
  memory[3] = 0x45

    0x12345 は20bit即値なので、16進数5桁です。

  imm = 0x12345

    このうち最初の 1 が rd と同じバイトに入ります。

  rd   = 0x7
  imm  = 0x12345

  rd と imm の先頭を合わせて:

  0x71

  なので2バイト目が 0x71 になります。

  0x 40 71 23 45
     ^^ ^^ ^^ ^^
     b0 b1 b2 b3


```
成功条件:

```text
MOVI R7, 0x12345
実行後 R7 = 0x12345
```

### Day 6: SYSCALL print_char

目的:

```text
VM内の値をhost端末へ1文字出す
```

仕様:

```text
SYSCALL 0
R0の下位8bitを文字として出力
```

成功条件:

```text
MOVI R0, 65
SYSCALL 0
HALT
```

出力:

```text
A
CPU halted.
```

### Day 7: SYSCALL print_string

目的:

```text
VM内メモリの0終端文字列を表示する
```

仕様:

```text
SYSCALL 1
R0 = 文字列先頭アドレス
memory[R0] から 0 byte まで表示
```

成功条件:

```text
Hello from Handmade OS
CPU halted.
```

手作りプログラム:

```text
MOVI R0, 0x10
SYSCALL 1
HALT
```

手作りメモリ配置:

```text
0x00000000: 40 00 00 10    MOVI R0, 0x10
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 01 00 00 00    HALT

0x00000010: 48 65 6C 6C 6F 20 66 72
0x00000018: 6F 6D 20 48 61 6E 64 6D
0x00000020: 61 64 65 20 4F 53 0A 00
```

確認すること:

```text
R0 = 0x10
memory[0x10] から 1 byte ずつ読む
0 byte が来たら表示を止める
```

### Day 8: LDB

目的:

```text
VM内メモリから1 byteを読み、レジスタへ入れる
```

仕様:

```text
LDB rd, [rs]
rd = destination register
rs = address register
memory[rs] の1 byteを読み、rdへ入れる
```

命令形式:

```text
bits 31..28: type = 3
bits 27..24: op = 0
bits 23..20: rd
bits 19..16: rs
bits 15..0 : unused = 0
```

手計算:

```text
LDB R0, [R1]

type = 3
op   = 0
rd   = 0
rs   = 1

0x30010000
  3 0 0 1 0 0 0 0
```

手作りプログラム:

```text
MOVI R1, 0x10
LDB R0, [R1]
SYSCALL 0
HALT
```

手作りメモリ配置:

```text
0x00000000: 40 10 00 10    MOVI R1, 0x10
0x00000004: 30 01 00 00    LDB R0, [R1]
0x00000008: 60 00 00 00    SYSCALL 0
0x0000000C: 01 00 00 00    HALT

0x00000010: 41
```

成功条件:

```text
A
CPU halted.
```

確認すること:

```text
R1 = 0x10
memory[0x10] = 0x41
LDB後 R0 = 0x41
SYSCALL 0でAが出る
```

### Day 9: STB

目的:

```text
レジスタの下位8bitをVM内メモリへ書く
```

仕様:

```text
STB [rd], rs
rd = address register
rs = source register
rsの下位8bitをmemory[regs[rd]]へ書く
```

手作りプログラム:

```text
MOVI R1, 0x20
MOVI R0, 65
STB [R1], R0
LDB R2, [R1]
```

成功条件:

```text
memory[0x20] = 0x41
LDB後 R2 = 0x41
```

確認すること:

```text
R1 = 0x20
R0 = 0x41
STB後 memory[0x20] = 0x41
LDBで読み直すと R2 = 0x41
```

### Day 10: LDDI

目的:

```text
VM内メモリから4 byteを読み、32bit値としてレジスタへ入れる
```

仕様:

```text
LDDI rd, address
rd = destination register
address = immediate address
memory[address] から4 byteを読み、rdへ入れる
```

確認すること:

```text
4 byteをどの順番で32bit値にするか
big-endianで読むなら 12 34 56 78 => 0x12345678
LDDI後、対象レジスタに期待値が入るか
```

成功条件:

```text
memory[0x10..0x13] = 12 34 56 78
LDDI R0, 0x10
実行後 R0 = 0x12345678
```

### Day 11: STDI

目的:

```text
レジスタの32bit値をVM内メモリへ4 byteで書く
```

仕様:

```text
STDI address, rs
address = immediate address
rs = source register
rsの32bit値をmemory[address]から4 byteへ書く
```

確認すること:

```text
32bit値をどの順番で4 byteに分けるか
big-endianで書くなら 0x12345678 => 12 34 56 78
STDI後、memoryの4 byteが期待通りか
```

成功条件:

```text
MOVI R0, 0x12345
STDI 0x10, R0
実行後 memory[0x10..0x13] = 00 01 23 45
```

### Day 12: INC / DEC

目的:

```text
レジスタの値を1増やす、または1減らす
```

仕様:

```text
INC rd
rd = rd + 1

DEC rd
rd = rd - 1
```

確認すること:

```text
対象レジスタだけが変わるか
PCはfetch時の+4だけか
条件フラグは最初は変えない
```

成功条件:

```text
MOVI R0, 10
INC R0
DEC R0
実行後 R0 = 10
```

### Day 13: MOV

目的:

```text
あるレジスタの値を別のレジスタへコピーする
```

仕様:

```text
MOV rd, rs
rd = rs
```

確認すること:

```text
コピー元rsは変わらない
コピー先rdだけがrsと同じ値になる
```

成功条件:

```text
MOVI R1, 0x41
MOV R0, R1
SYSCALL 0
HALT
```

出力:

```text
A
CPU halted.
```

### Day 14: PUSH

目的:

```text
レジスタの値をスタックへ積む
```

仕様:

```text
PUSH rs
SPを動かす
rsの32bit値をmemory[SP]へ書く
```

確認すること:

```text
スタックは上に伸びるか、下に伸びるか
SPを先に動かすか、後に動かすか
32bit値をmemoryへ書くbyte順はSTDIと同じか
```

成功条件:

```text
MOVI R0, 0x12345
PUSH R0
SPとmemory[SP..SP+3]が仕様通りになる
```

### Day 15: POP

目的:

```text
スタックから32bit値を取り出してレジスタへ入れる
```

仕様:

```text
POP rd
memory[SP]から32bit値を読む
SPを動かす
rdへ値を入れる
```

確認すること:

```text
PUSHと逆向きにSPが戻るか
32bit値をmemoryから読むbyte順はLDDIと同じか
PUSHした値をPOPで取り戻せるか
```

成功条件:

```text
MOVI R0, 0x12345
PUSH R0
POP R1
実行後 R1 = 0x12345
SPが元の位置に戻る
```

## How To Read Existing Code

既存コードは、最初に読むものではなく、照合に使う。

読む順番:

```text
1. 自分の仕様カードを書く
2. 自分で実装する
3. 動かす
4. ログを見る
5. まだ分からなければ既存コードを見る
```

見る範囲を限定する。

- `lc3.c`: 端末処理、fetch/decode/execute ループの雰囲気
- `emu.py`: 独自CPUの命令仕様、syscall仕様
- `asm.py`: 命令のエンコード方法
- shell tutorial: コマンドループ、入力分割、プロセス起動の考え方
- kilo: 端末制御、バッファ、編集操作、保存処理の考え方

## Debugging Checklist

動かないときは、次の順で見る。

```text
1. PCは期待したアドレスを指しているか
2. fetchした32bit値は期待通りか
3. type/opのdecodeは合っているか
4. rdやimmの取り出しは合っているか
5. レジスタに書かれた値は合っているか
6. syscall番号は合っているか
7. 文字列アドレスは合っているか
8. 文字列は0終端されているか
```

ログ例:

```text
PC=00000000 INST=40780010 TYPE=4 OP=0
R7=80010
PC=00000004 INST=60000001 TYPE=6 OP=0
SYSCALL=1 ARG0=80010
```

## How To Ask For Help

完成コードを出さずに進めたいときは、次のように依頼する。

```text
次のステップの仕様カードだけ作ってください。完成コードは書かないでください。
```

```text
この実装をレビューしてください。答えのコードは出さず、ヒントだけください。
```

```text
この命令のbit配置が合っているか確認してください。
```

```text
このバグの原因候補を3つだけ挙げてください。修正コードはまだ不要です。
```

## Long-Term Layers

### CPU

最初は小さく始める。

```text
HALT
MOVI
SYSCALL
LDB/STB
LDD/STD
PUSH/POP
CALL/RET
JUMP
conditional jump
interrupt
MMU
page fault
```

### OS

最初はOSというより、CPU上で動く常駐プログラムとして始める。

```text
boot message
command loop
syscall table
file loading
task management
virtual console
```

### Shell

最初は `os.asm` 内蔵でよい。

```text
read line
split tokens
dispatch command
run external program
```

後で `shell.bin` として分離する。

### Editor

最初は kilo 風の最小エディターを目指す。

```text
screen clear
cursor movement
text buffer
insert char
delete char
save file
quit
```

## Rule For Progress

1日で完成させようとしない。

毎回の成功条件は、必ず小さくする。

よい成功条件:

```text
HALTだけ動く
MOVIでR7だけ変わる
SYSCALL 0でAだけ出る
文字列1つだけ出る
1文字だけ入力できる
```

悪い成功条件:

```text
OSを全部動かす
kiloを移植する
シェルを完成させる
ファイルシステムを作る
```

小さな成功条件を積み上げる。


| type番号 | 分類（系） | 英語名 | 具体的な処理内容の例 |
| :---: | :--- | :--- | :--- |
| 1 | ALU / 算術演算系 | Arithmetic | `ADD`（足し算）、`SUB`（引き算）、`MUL`（かけ算）など |
| 2 | 論理演算系 | Logical | `AND`（論理積）、`OR`（論理和）、`XOR`（排他的論理和）など |
| 3 | シフト演算系 | Shift | `SLL`（左シフト）、`SRL`（右シフト：ビットを左右にずらす処理） |
| 4 | MOVI / データ移動系 | Move / Load | `MOVI`（数字をレジスタに直接入れる）、`MOV`、`LD`（ロード） |
| 5 | ストア系 | Store | `ST`（レジスタのデータをメモリに書き戻して保存する） |
| 6 | 分岐 / ジャンプ系 | Branch / Jump | `JMP`（指定のアドレスへ飛ぶ）、`BEQ`（条件が合致したら飛ぶ） |
| 7 | システム / 特殊命令系 | System | `NOP`（何もしない）、`HALT`（停止）、`SYSCALL`（システム呼び出し） |

### 📝 命令コード `0x40000041` の構造解析

メモリに入っている4バイト（32ビット）のデータ「`0x40000041`」を、CPUは以下のように分解して処理しています。

| ビット位置（16進数） | 該当データ | 役割・意味 | 具体的な指示内容 |
| :---: | :---: | :--- | :--- |
| 先頭（左端） | `4` | `type = 4`（大分類） | 「MOVI（即値代入）系」の命令である |
| 中央 | `00000` | レジスタ指定 | データを格納する箱は「`R0`」である |
| 末尾（右端） | `41` | 即値データ（小分類） | 代入する値は「`0x41`（10進数で 65）」である |

【実行される命令】 `MOVI R0, 65` （レジスタR0に数値65を直接代入せよ）

　# 0x40000041

 memory[0x00000000] = 0x40;  // MOVI R0, 65
 memory[0x00000001] = 0x00;
 memory[0x00000002] = 0x00;
 memory[0x00000003] = 0x41;

0x41 == 41 * 16 == 65

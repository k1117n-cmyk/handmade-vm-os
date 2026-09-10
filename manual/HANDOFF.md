# 新しい命令やVM機能を追加するときの引き継ぎ書

このファイルは、新しい命令やVM機能を追加するときに、次回以降の作業で迷わないようにまとめたものです。

## 基本方針

新しい命令を追加するときは、次の4点をそろえる。

```text
1. 仕様書
2. テストコード
3. テストコード解説
4. 命令一覧・補助資料の更新
```

コードだけ先に増やすのではなく、仕様、テスト、解説、索引を同時に更新する。

Day 15 `POP` 以降の追加順は [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) を基準にする。現在は Day 22 `hello.bin` 作成ツールまで完了している。

今後の基本フローは [../HANDWRITING_GUIDE.md](../HANDWRITING_GUIDE.md) の `Test Flow` を基準にする。

```text
1. 仕様カードを書く
2. notes/NNN-name-test.c で部品として小さく確認する
3. 問題なければ notes/vm.c へ統合する
4. programs/*.bin をVMから起動して確認する
5. README と manual の索引を更新する
```

作業終わりには [DOC_UPDATE_CHECKLIST.md](DOC_UPDATE_CHECKLIST.md) を見て、周辺ファイルの更新漏れを確認する。

## 追加するファイル

Day番号と命令名に合わせて、次の3ファイルを追加する。

```text
manual/specs/010-lddi.md
notes/010-lddi-test.c
manual/test-code-explanations/010-lddi-test.md
```

次の命令なら、番号と命令名を変える。

```text
manual/specs/011-stdi.md
notes/011-stdi-test.c
manual/test-code-explanations/011-stdi-test.md
```

## 仕様書に書くこと

仕様書では、あとでテストコードを書けるだけの条件を書く。

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

特に大事なのは次の項目。

```text
どのbitをどう読むか
どのレジスタを読むか
どのレジスタへ書くか
メモリを読む/書くなら、アドレスはどこから来るか
成功したと言える条件は何か
```

## 命令番号を決める

既存の命令一覧は [instruction-types.md](instruction-types.md) を確認する。

現在の主な割り当て:

```text
HALT    inst == 0x01000000
RET     inst == 0x02000000
MOV     type=1, op=0
INC     type=2, op=0
DEC     type=2, op=1
ADD     type=2, op=2
SUB     type=2, op=3
CMP     type=2, op=4
LDB     type=3, op=0
STB     type=3, op=1
MOVI    type=4, op=0
LDDI    type=5, op=0
STDI    type=5, op=1
SYSCALL type=6, op=0
JUMP    type=6, op=8
CALLI   type=6, op=9
JZ      type=6, op=10
JNZ     type=6, op=11
PUSH    type=7, op=0
POP     type=7, op=1
```

未使用の `type` / `op` を使う。似た命令は同じ分類に寄せる。

例:

```text
LDDI: immediate addressでmemoryから読む
=> direct memory instruction
=> type=5, op=0

STDI: immediate addressでmemoryへ書く
=> direct memory instruction
=> type=5, op=1

INC/DEC: registerの値を1つ増減する
=> arithmetic-register instruction
=> type=2, op=0/1

MOV: registerからregisterへコピーする
=> register instruction
=> type=1, op=0

PUSH: registerの値をstackへ積む
=> stack instruction
=> type=7, op=0

POP: stackからregisterへ値を取り出す
=> stack instruction
=> type=7, op=1

ADD/SUB: register同士で計算する
=> arithmetic-register instruction
=> type=2, op=2/3

CMP: register同士を比較してzero_flagを更新する
=> arithmetic-register instruction
=> type=2, op=4

JUMP/JZ/JNZ: immediate addressへPCを変更する
=> control flow instruction
=> type=6, op=8/10/11

CALLI: return addressをstackへ積んでimmediate addressへPCを変更する
=> control flow instruction
=> type=6, op=9

RET: stackからreturn addressを取り出してPCへ戻す
=> system / control flow instruction
=> inst == 0x02000000
```

## テストコードを書く

テストコードは `notes/` に置く。

```text
notes/010-lddi-test.c
```

テストコードは、できるだけ1つの命令を小さく確認する。

LDDIの例:

```asm
LDDI R0, 0x10
HALT
```

成功条件:

```text
memory[0x10..0x13] = 12 34 56 78
実行後 R0 = 0x12345678
```

テストコードには、最低限これを入れる。

```text
VM状態の初期化
memoryへの命令配置
必要なテストデータ配置
fetch
decode
execute
結果表示
成功/失敗判定
```

## テストコード解説を書く

テストコード解説は `manual/test-code-explanations/` に置く。

```text
manual/test-code-explanations/010-lddi-test.md
```

解説では、次の順番で書くと読みやすい。

```text
1. このテストの目的
2. 実行する命令列
3. VMの状態
4. メモリ配置
5. 命令の読み取り
6. 命令フォーマット
7. 対象命令の実行
8. エラーチェック
9. HALT
10. 最後の判定
```

テストコード解説は、仕様書より詳しく、Cコードを読む人向けに書く。

## 更新する既存ファイル

新しい命令を追加したら、必要に応じて次のファイルを更新する。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

### manual/instruction-types.md

命令一覧に追加する。

```text
Type一覧
命令一覧
命令値の例
Decodeの流れ
```

### manual/README.md

現在の命令一覧に追加する。

### manual/reference/instruction-fields.md

fieldの読み方として必要なら、命令の対応例を追加する。

全面的に詳しくしすぎない。詳しい仕様は `manual/specs/` に任せる。

### README.md

リポジトリ全体の説明に影響がある場合だけ更新する。

例:

```text
扱う命令一覧
notes/ のファイル一覧
manual/ への案内
```

### notes/vm.c

統合VMにも命令の実行分岐を追加する。

ただし、テストプログラム自体を新命令用に差し替えるかどうかは、その時点の学習ステップに合わせて決める。

## 実装時の注意

### big-endianをそろえる

このVMでは、命令fetchも複数byteの値の読み書きもbig-endianでそろえる。

4 byteを32bit値にする例:

```c
uint32_t value =
    ((uint32_t)memory[address] << 24) |
    ((uint32_t)memory[address + 1] << 16) |
    ((uint32_t)memory[address + 2] << 8) |
    ((uint32_t)memory[address + 3]);
```

### メモリ範囲チェック

1 byte読む命令なら、対象アドレスが `sizeof(memory)` 未満か確認する。

```c
address < sizeof(memory)
```

4 byte読む命令なら、`address + 3` まで範囲内か確認する。

```c
address <= sizeof(memory) - 4
```

コードではオーバーフローや境界を避けるため、次の形にする。

```c
if (address > sizeof(memory) - 4) {
    printf("memory address out of range: 0x%08X\n", address);
    running = false;
}
```

### レジスタ範囲チェック

今のVMの通常レジスタは `R0` から `R7`。

```c
if (rd >= 8 || rs >= 8) {
    printf("invalid register: rd=R%u rs=R%u\n", rd, rs);
    running = false;
}
```

`rs` を使わない命令では `rd` だけ確認する。

## 動作確認

新しいテストコードをコンパイルして実行する。

```sh
cc notes/010-lddi-test.c -o /tmp/010-lddi-test
/tmp/010-lddi-test
```

`notes/vm.c` も更新した場合は、こちらも確認する。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm
```

## Git確認

作業前後で未コミット変更を確認する。

```sh
git status --short
```

ユーザーが先に変更していたファイルは勝手に戻さない。

今回のDay 10作業では、開始時点で `HANDWRITING_GUIDE.md` に未コミット変更があったため、そこは触らずに作業した。

## Day 10で実際に行ったこと

Day 10 `LDDI` では、次を追加した。

```text
manual/specs/010-lddi.md
notes/010-lddi-test.c
manual/test-code-explanations/010-lddi-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R0=0x12345678
LDDI test passed.
```

`notes/vm.c` の確認結果:

```text
A
CPU halted.
```

## Day 11で実際に行ったこと

Day 11 `STDI` では、次を追加した。

```text
manual/specs/011-stdi.md
notes/011-stdi-test.c
manual/test-code-explanations/011-stdi-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R0=0x12345678
memory[0x10..0x13]=12 34 56 78
STDI test passed.
```

`notes/vm.c` の確認結果:

```text
A
CPU halted.
```

## Day 12で実際に行ったこと

Day 12 `INC` / `DEC` では、次を追加した。

```text
manual/specs/012-inc-dec.md
notes/012-inc-dec-test.c
manual/test-code-explanations/012-inc-dec-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R0=0x00000011
R1=0x0000000F
INC/DEC test passed.
```

`notes/vm.c` の確認結果:

```text
A
CPU halted.
```

## Day 13で実際に行ったこと

Day 13 `MOV` では、次を追加した。

```text
manual/specs/013-mov.md
notes/013-mov-test.c
manual/test-code-explanations/013-mov-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R0=0x00012345
R2=0x00012345
MOV test passed.
```

`notes/vm.c` の確認結果:

```text
A
CPU halted.
```

## Day 14で実際に行ったこと

Day 14 `PUSH` では、次を追加した。

```text
manual/specs/014-push.md
notes/014-push-test.c
manual/test-code-explanations/014-push-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R0=0x12345678
SP=0x000FFFFC
memory[SP..SP+3]=12 34 56 78
PUSH test passed.
```

`notes/vm.c` の確認結果:

```text
A
CPU halted.
```

## Day 15で実際に行ったこと

Day 15 `POP` では、次を追加した。

```text
manual/specs/015-pop.md
notes/015-pop-test.c
manual/test-code-explanations/015-pop-test.md
```

次を更新した。

```text
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
R1=0x12345678
SP=0x00100000
POP test passed.
```

## Day 16からDay 20で実際に行ったこと

Day 16からDay 20では、次を追加した。

```text
manual/specs/016-add-sub.md
manual/specs/017-cmp.md
manual/specs/018-jump-jz.md
manual/specs/019-jnz.md
manual/specs/020-calli-ret.md

notes/016-add-sub-test.c
notes/017-cmp-test.c
notes/018-jump-jz-test.c
notes/019-jnz-test.c
notes/020-calli-ret-test.c

manual/test-code-explanations/016-add-sub-test.md
manual/test-code-explanations/017-cmp-test.md
manual/test-code-explanations/018-jump-jz-test.md
manual/test-code-explanations/019-jnz-test.md
manual/test-code-explanations/020-calli-ret-test.md
```

次を更新した。

```text
HANDWRITING_GUIDE.md
manual/instruction-types.md
manual/README.md
manual/reference/instruction-fields.md
README.md
notes/vm.c
```

確認した実行結果:

```text
notes/*-test.c と notes/vm.c をすべて cc でコンパイル・実行
結果: all ok
```

`notes/vm.c` の確認結果:

```text
A
VM flow complete.
CPU halted.
```

## Day 21で実際に行ったこと

Day 21 `Binary Loader` では、次を追加した。

```text
manual/specs/021-binary-loader.md
notes/021-binary-loader-test.c
manual/test-code-explanations/021-binary-loader-test.md
```

次を更新した。

```text
README.md
manual/README.md
manual/instruction-types.md
notes/vm.c
```

`notes/vm.c` は、引数なしなら従来の内蔵テストプログラムを実行し、引数ありなら外部バイナリを `memory[0]` から読み込んで実行する。

確認した実行結果:

```text
cc notes/021-binary-loader-test.c -o /tmp/021-binary-loader-test
/tmp/021-binary-loader-test

inst0=0x40000041
inst1=0x60000000
inst2=0x01000000
binary loader test passed.
```

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm /tmp/021-program.bin

A
CPU halted.
```

## programs/hello.binで実際に行ったこと

外部バイナリローダーをリポジトリ内の実ファイルで確認できるように、次を追加した。

```text
programs/README.md
programs/hello.bin
```

`programs/hello.bin` の中身:

```text
40 00 00 41
60 00 00 00
01 00 00 00
```

対応する命令:

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

確認した実行結果:

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin

A
CPU halted.
```

## Day 22で実際に行ったこと

Day 22 `Hello Binary Writer` では、`programs/hello.bin` を再生成できる小さな作成ツールを追加した。

```text
tools/write-hello-bin.c
manual/specs/022-hello-binary-writer.md
manual/test-code-explanations/022-hello-binary-writer.md
```

次を更新した。

```text
README.md
manual/README.md
programs/README.md
```

確認した実行結果:

```text
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin

wrote programs/hello.bin
```

```text
xxd programs/hello.bin

00000000: 4000 0041 6000 0000 0100 0000            @..A`.......
```

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin

A
CPU halted.
```

## 現在の方針

今回の企画は、既存教材を組み合わせるのではなく、自作CPU、自作VM、自作OS、自作シェル、自作エディターを一から手書きで組み上げる。

既存教材は、最初に読むものではなく、迷ったときや実装後に照合する参考資料として扱う。

今後の作業は、次の順番を守る。

```text
1. 仕様カードを書く
2. notes/NNN-name-test.c で部品として小さく確認する
3. 問題なければ notes/vm.c へ統合する
4. programs/*.bin をVMから起動して確認する
5. README と manual の索引を更新する
```

`/tmp` はコンパイル結果や一時テストファイルに使う。

学習用に残したい実行サンプルは `programs/` に置く。

## 次回の候補

次回は、次のどちらかから選ぶ。

```text
候補A: 小さな assembler へ進む
候補B: 入力系 SYSCALL の仕様カードを書く
```

現時点では、候補Aを優先するのが自然。

理由は、外部バイナリローダーと `hello.bin` 作成ツールまで進んだので、次は手で命令値を書く段階から、小さなアセンブリ表記をバイナリへ変換する段階へ進めるため。

ただし、本格的なアセンブラにはしない。最初は次の3命令だけでよい。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

最初の成功条件:

```text
tools/small-asm.c または tools/small-asm.py で programs/hello.bin 相当を生成する
生成した .bin を notes/vm.c で実行する

A
CPU halted.
```

もし入力系へ進む場合は、いきなり行編集やOS風コマンドループへ進まない。

最初の成功条件は次の程度にする。

```text
1文字だけ読む
読んだ文字をR0へ入れる
SYSCALL 0で同じ文字を表示する
```

## 再開時に確認すること

作業再開時は、まず次を確認する。

```sh
git status --short
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待出力:

```text
A
CPU halted.
```

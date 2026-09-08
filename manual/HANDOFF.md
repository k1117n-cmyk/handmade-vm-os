# 新しい命令を追加するときの引き継ぎ書

このファイルは、Day 10 `LDDI` を追加したときの作業手順を、次回以降の命令追加で迷わないようにまとめたものです。

## 基本方針

新しい命令を追加するときは、次の4点をそろえる。

```text
1. 仕様書
2. テストコード
3. テストコード解説
4. 命令一覧・補助資料の更新
```

コードだけ先に増やすのではなく、仕様、テスト、解説、索引を同時に更新する。

Day 15 `POP` 以降の追加順は [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) を基準にする。ブログ記事とのつながりを優先し、まず `ADD` / `SUB`、`CMP`、`JUMP` / `JZ` の順で進める。

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

## 記事第3回の作成引き継ぎ

次回は、記事第3回を仕上げるところから再開する。

対象の下書き:

```text
articles/2026-09-08-vm-control-flow-016-020-outline.md
```

このファイルは `articles/` 配下にあり、`.gitignore` によりGit管理対象外。記事本文の編集内容はローカル下書きとして扱う。

参照するスタイルガイド:

```text
articles/ARTICLE_STYLE_GUIDE.md
```

参考記事:

```text
articles/unix-cafe-handmade-vm-02.txt
```

ユーザーの希望:

```text
一旦記事を仕上げたい
ARTICLE_STYLE_GUIDE.md に従う
初心者に分かりやすい、自然な日本語にする
第2回の記事の説明が分かりやすかったので、その説明調に寄せる
```

次回やること:

```text
1. articles/ARTICLE_STYLE_GUIDE.md を読む
2. articles/unix-cafe-handmade-vm-02.txt を参考に、WordPressブロック形式の文体を確認する
3. articles/2026-09-08-vm-control-flow-016-020-outline.md を完成記事へ書き換える
4. Day 16からDay 20を、初心者向けに1章ずつ説明する
5. 公開用メモのスラッグ、タグ、ディスクリプション、アイキャッチ用プロンプトを末尾に残す
```

記事に入れる主な流れ:

```text
導入:
  前回までで、値を置く、読む、書く、スタックへ退避するところまで進んだ
  今回は、上から順番に実行するだけのVMから一歩進める

Day 16 ADD / SUB:
  レジスタ同士で計算する
  ADD R0, R1 は regs[0] = regs[0] + regs[1]
  SUB R2, R3 は regs[2] = regs[2] - regs[3]

Day 17 CMP:
  レジスタを書き換えず、比較結果だけを zero_flag に残す
  次の JZ / JNZ がこの結果を使う

Day 18 JUMP / JZ:
  PCを書き換えて実行位置を変える
  fetchでPCが一度+4されたあと、必要ならPCをジャンプ先へ上書きする

Day 19 JNZ:
  JZの逆で、zero_flagがfalseのときにジャンプする
  CMP、JZ、JNZでif文に近い形が見えてくる

Day 20 CALLI / RET:
  CALLIは戻り先PCをスタックに積んでからジャンプする
  RETはスタックから戻り先PCを取り出して戻る

SYSCALL 1:
  R0を文字列アドレスとして使い、0終端まで表示する
  PCは命令の場所、R0はデータの場所として分けて説明する

まとめ:
  ADD/SUBで計算
  CMPで比較
  JUMP/JZ/JNZで分岐
  CALLI/RETでサブルーチン
  SYSCALL 1で文字列表示
```

記事化するときの注意:

```text
地の文は「です・ます」調にする
命令名、ファイル名、変数名はバッククォートで囲む
仕様カードからCコードへ直接飛ばず、アセンブリ例、手計算、fetch/decode/executeの順に挟む
命令ごとに全部の仕様カードを長く載せすぎず、読者が止まりやすい点を優先する
JUMP/JZ/JNZでは、fetch後にPCが+4されてからPCを上書きする点を必ず説明する
CALLI/RETでは、戻り先PCが「CALLIの次の命令アドレス」になる点を必ず説明する
```

作業開始時点の状態:

```text
記事本文の書き換えは未着手
git status は clean
直近のGitHub反映済みコミットは bc77a86 Stop tracking article drafts
articles/ はGit管理対象外
```

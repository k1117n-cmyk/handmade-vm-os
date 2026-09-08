# 今後の命令追加ガイドライン

この文書は、Day 15 `POP` まで進んだ後に、次の命令をどの順番で追加するかを整理するためのガイドラインです。

ブログ記事では、次に実装する命令候補として `ADD`, `SUB`, `CMP`, `JUMP`, `JZ` を挙げている。

一方で、長期ロードマップでは lesson 04 の `os.asm` を動かすために、`CALLI`, `RET`, `JPI`, `JPZI`, `JPNZI`, `JPUI`, `JPNUI` も必要になる。

この2つは矛盾ではなく、目的が違う。

```text
ブログの学習順:
  計算する
  比較する
  分岐する
  ループやif文の形を作る

OS起動の実装順:
  既存 os.asm が使っている命令をそろえる
  関数呼び出しと戻り
  条件分岐
  syscall拡張
```

## 基本方針

次の数回は、ブログ記事とのつながりを優先する。

Day 16 以降は、まず `ADD`, `SUB`, `CMP`, `JUMP`, `JZ` の流れで進める。

理由は、ここで「CPUが上から順番に実行するだけの機械」から、「条件によって流れを変えられる機械」へ変わるから。

```text
MOVI / MOV / LDB / STB / PUSH / POP:
  値を置く、読む、書く、退避する

ADD / SUB:
  値を計算する

CMP:
  値を比べて条件フラグを作る

JUMP / JZ:
  PCを書き換えて実行順序を変える
```

## 推奨する次の順番

### Day 16: ADD / SUB

まずはレジスタ同士の足し算と引き算を追加する。

```asm
ADD R0, R1
SUB R0, R1
```

成功条件は小さくする。

```text
R0 = 10
R1 = 3
ADD R0, R1
=> R0 = 13

R0 = 10
R1 = 3
SUB R0, R1
=> R0 = 7
```

この段階では、条件分岐用のフラグ更新は必須にしない。必要なら、計算結果が `0` のときだけ `zero flag` を立てるところまでに留める。

### Day 17: CMP と zero flag

次に、値を比較する命令を追加する。

```asm
CMP R0, R1
```

`CMP` はレジスタの値を直接書き換えない。内部的には `R0 - R1` のように比較し、その結果を条件フラグへ反映する。

最初に使うフラグは `zero flag` だけでよい。

```text
R0 == R1 なら zero flag = 1
R0 != R1 なら zero flag = 0
```

このリポジトリの学習VMでは、まず `VM` 構造体に `zero_flag` を追加する方針にする。

```c
bool zero_flag;
```

元教材の `emu.py` は `CR` レジスタの下位ビットで Zero / Overflow / Underflow を管理しているが、最初から `CR` 全体を導入しない。ブログ向けには `zero_flag` を明示した方が読みやすい。

### Day 18: JUMP / JZ

次に、PCを書き換える命令を追加する。

```asm
JUMP 0x20
JZ 0x20
```

`JUMP` は無条件に `PC` を変更する。

`JZ` は `zero_flag == true` のときだけ `PC` を変更する。

この2つが入ると、次のような流れを作れる。

```asm
MOVI R0, 3
loop:
DEC R0
CMP R0, R1
JZ end
JUMP loop
end:
HALT
```

## 名前の使い分け

ブログでは、読者に伝わりやすい名前として `JUMP` / `JZ` を使ってよい。

ただし、lesson 04 の教材側では次の名前が使われている。

```text
JPI    immediate addressへ無条件ジャンプ
JPZI   zero flagが立っていたら immediate addressへジャンプ
JPNZI  zero flagが立っていなければ immediate addressへジャンプ
JPUI   underflow flagが立っていたら immediate addressへジャンプ
JPNUI  underflow flagが立っていなければ immediate addressへジャンプ
CALLI  return addressをpushして immediate addressへジャンプ
RET    stackからreturn addressをpopしてPCへ戻す
```

今後の文書では、次の対応にする。

```text
ブログ上の名前    VM内部またはOS教材寄りの名前
JUMP              JPI
JZ                JPZI
JNZ               JPNZI
CALL              CALLI
RET               RET
```

ブログで `JUMP` / `JZ` と書く場合でも、仕様書では「現在のVMでは immediate address を使うので、内部的には `JPI` / `JPZI` 相当」と明記する。

## CALLI / RET の扱い

`CALLI` / `RET` は重要だが、`ADD`, `SUB`, `CMP`, `JUMP`, `JZ` より後に回してよい。

理由は、関数呼び出しは次の要素を同時に理解する必要があるため。

```text
PC
return address
stack
PUSH / POP
関数の入口と出口
```

先に `JUMP` / `JZ` で「PCを書き換える」ことに慣れてから、`CALLI` / `RET` へ進む。

推奨順は次の通り。

```text
Day 16: ADD / SUB
Day 17: CMP / zero flag
Day 18: JUMP / JZ
Day 19: JNZ
Day 20: CALLI / RET
Day 21: lesson 04 用の JPZI / JPNZI / JPUI / JPNUI 整理
```

## 条件フラグの段階的な扱い

最初に入れるフラグは `zero_flag` だけにする。

```text
zero_flag:
  直前の比較結果が0なら true
  それ以外なら false
```

その後、lesson 04 の `JPUI` / `JPNUI` が必要になった時点で `underflow_flag` を追加する。

```text
underflow_flag:
  引き算の結果が0未満になったら true
  それ以外なら false
```

`overflow_flag` と `CR` レジスタは、さらに後でよい。

```text
第1段階: bool zero_flag
第2段階: bool underflow_flag
第3段階: flags構造体
第4段階: CRレジスタへ統合
```

## 新しい命令を追加するときの作業単位

今後も、1回の追加では次の4点をそろえる。

```text
1. manual/specs/NNN-name.md
2. notes/NNN-name-test.c
3. manual/test-code-explanations/NNN-name-test.md
4. manual/instruction-types.md と manual/README.md の更新
```

統合VM `notes/vm.c` へ入れるのは、個別テストが通った後にする。

ブログ記事では、すべての内部事情を一度に説明しない。各回の主題に関係する分だけ説明する。

```text
ADD / SUB回:
  レジスタ同士の計算に集中する

CMP回:
  比較とzero flagに集中する

JUMP / JZ回:
  PC変更と条件分岐に集中する

CALLI / RET回:
  return addressとstackに集中する
```

## 判断ルール

迷ったら、次の優先順位で決める。

```text
1. ブログ記事の読者が前回から自然に理解できる順番か
2. 1命令または1概念だけを小さくテストできるか
3. 既存の `notes/vm.c` に無理なく統合できるか
4. lesson 04 の `os.asm` 起動へ近づくか
```

短期的にはブログの学習順を優先する。

中期的には lesson 04 の `os.asm` 起動に戻る。

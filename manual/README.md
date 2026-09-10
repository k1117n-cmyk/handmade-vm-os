# Handmade VM Manual

このディレクトリは、VMの仕様、補助資料、テストコード解説をまとめたマニュアルです。

## 読む順番

初めて読むときは、次の順番がおすすめです。

```text
1. instruction-types.md
2. reference/instruction-fields.md
3. specs/
4. test-code-explanations/
```

## ファイルの役割

| 場所 | 役割 |
|---|---|
| [instruction-types.md](instruction-types.md) | このVM専用の命令タイプと命令一覧 |
| [HANDOFF.md](HANDOFF.md) | 新しい命令を追加するときの引き継ぎ書 |
| [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) | Day 15以降の命令追加順と、ブログ向け学習順の整理 |
| [specs/](specs/) | 命令ごとの仕様書 |
| [reference/](reference/) | fieldの読み方や、一般的な命令分類の補助資料 |
| [test-code-explanations/](test-code-explanations/) | `notes/*-test.c` を読むための解説 |

## 使い分け

命令の種類を探したいときは [instruction-types.md](instruction-types.md) を見ます。

新しい命令を追加するときは [HANDOFF.md](HANDOFF.md) を見ます。

Day 15 `POP` 以降の命令追加順や、次の節目である外部バイナリローダーの方針を確認したいときは [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) を見ます。

`type`, `op`, `rd`, `rs`, `imm` の意味を確認したいときは [reference/instruction-fields.md](reference/instruction-fields.md) を見ます。

ある命令の仕様を確認したいときは [specs/](specs/) を見ます。

テストコードが何をしているか確認したいときは [test-code-explanations/](test-code-explanations/) を見ます。

## 現在の命令

| 命令 | 概要 |
|---|---|
| `HALT` | VMを停止する |
| `RET` | スタックから戻り先PCを取り出して戻る |
| `MOV` | レジスタの値を別のレジスタへコピーする |
| `INC` | レジスタの値を1増やす |
| `DEC` | レジスタの値を1減らす |
| `ADD` | レジスタに別のレジスタの値を足す |
| `SUB` | レジスタから別のレジスタの値を引く |
| `CMP` | 2つのレジスタを比較してzero flagを更新する |
| `MOVI` | 即値をレジスタへ入れる |
| `SYSCALL` | VM外側のサービスを呼ぶ |
| `JUMP` | PCを即値アドレスへ変更する |
| `CALLI` | 戻り先PCをスタックへ積んで即値アドレスへ移動する |
| `JZ` | zero flagが立っているときだけPCを即値アドレスへ変更する |
| `JNZ` | zero flagが立っていないときだけPCを即値アドレスへ変更する |
| `LDB` | メモリから1 byte読み込む |
| `STB` | メモリへ1 byte書き込む |
| `LDDI` | メモリから4 byte読み込む |
| `STDI` | メモリへ4 byte書き込む |
| `PUSH` | レジスタの32bit値をスタックへ積む |
| `POP` | スタックから32bit値をレジスタへ取り出す |

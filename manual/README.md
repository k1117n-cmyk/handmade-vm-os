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
| [specs/](specs/) | 命令ごとの仕様書 |
| [reference/](reference/) | fieldの読み方や、一般的な命令分類の補助資料 |
| [test-code-explanations/](test-code-explanations/) | `notes/*-test.c` を読むための解説 |

## 使い分け

命令の種類を探したいときは [instruction-types.md](instruction-types.md) を見ます。

新しい命令を追加するときは [HANDOFF.md](HANDOFF.md) を見ます。

`type`, `op`, `rd`, `rs`, `imm` の意味を確認したいときは [reference/instruction-fields.md](reference/instruction-fields.md) を見ます。

ある命令の仕様を確認したいときは [specs/](specs/) を見ます。

テストコードが何をしているか確認したいときは [test-code-explanations/](test-code-explanations/) を見ます。

## 現在の命令

| 命令 | 概要 |
|---|---|
| `HALT` | VMを停止する |
| `MOVI` | 即値をレジスタへ入れる |
| `SYSCALL` | VM外側のサービスを呼ぶ |
| `LDB` | メモリから1 byte読み込む |
| `STB` | メモリへ1 byte書き込む |
| `LDDI` | メモリから4 byte読み込む |

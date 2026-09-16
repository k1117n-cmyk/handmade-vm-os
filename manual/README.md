# Handmade VM Manual

このディレクトリは、VMの仕様、補助資料、テストコード解説をまとめたマニュアルです。

## 読む順番

初めて読むときは、次の順番がおすすめです。

```text
1. instruction-types.md
2. reference/instruction-fields.md
3. specs/
4. test-code-explanations/
5. HANDOFF.md
```

## ファイルの役割

| 場所 | 役割 |
|---|---|
| [instruction-types.md](instruction-types.md) | このVM専用の命令タイプと命令一覧 |
| [HANDOFF.md](HANDOFF.md) | 次回作業を再開するための短いメモ |
| [WORKLOG.md](WORKLOG.md) | 過去に実際に行った作業ログ |
| [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) | 今後の命令やsyscall追加方針 |
| [DOC_UPDATE_CHECKLIST.md](DOC_UPDATE_CHECKLIST.md) | 作業終わりに周辺ファイルの更新漏れを確認するチェックリスト |
| [specs/](specs/) | 命令ごとの仕様書 |
| [reference/](reference/) | fieldの読み方や、一般的な命令分類の補助資料 |
| [test-code-explanations/](test-code-explanations/) | `notes/*-test.c` や `tools/*` を読むための解説 |

## VM起動機能

| 機能 | 概要 | 仕様 |
|---|---|---|
| 外部バイナリローダー | `program.bin` を `memory[0]` から読み込む | [specs/021-binary-loader.md](specs/021-binary-loader.md) |
| hello.bin 作成ツール | 最小バイナリを生成する | [specs/022-hello-binary-writer.md](specs/022-hello-binary-writer.md) |
| small assembler | 最小アセンブリを `.bin` へ変換する | [specs/023-small-asm.md](specs/023-small-asm.md) |
| echo-char sample | 入力した1文字をそのまま表示する外部プログラム | [specs/025-echo-char.md](specs/025-echo-char.md) |
| prompt-echo sample | `>` を表示してから入力した1文字を表示する外部プログラム | [specs/026-prompt-echo.md](specs/026-prompt-echo.md) |

Day 22 と Day 23 は開発補助ツールの追加なので、対応するCコードは `notes/` ではなく `tools/` に置きます。

Day 25 はサンプルプログラムの追加なので、対応するコードは `notes/` ではなく `programs/echo-char.asm` と `programs/echo-char.bin` に置きます。解説は [test-code-explanations/025-echo-char.md](test-code-explanations/025-echo-char.md) にあります。

Day 26 もサンプルプログラムの追加なので、対応するコードは `notes/` ではなく `programs/prompt-echo.asm` と `programs/prompt-echo.bin` に置きます。解説は [test-code-explanations/026-prompt-echo.md](test-code-explanations/026-prompt-echo.md) にあります。

## 使い分け

命令の種類を探したいときは [instruction-types.md](instruction-types.md) を見ます。

作業を再開するときは [HANDOFF.md](HANDOFF.md) を見ます。

過去に何を追加したか確認したいときは [WORKLOG.md](WORKLOG.md) を見ます。

今後の命令やsyscall追加方針を確認したいときは [NEXT_INSTRUCTION_GUIDELINES.md](NEXT_INSTRUCTION_GUIDELINES.md) を見ます。

作業終わりに更新漏れを確認するときは [DOC_UPDATE_CHECKLIST.md](DOC_UPDATE_CHECKLIST.md) を見ます。

`type`, `op`, `rd`, `rs`, `imm` の意味を確認したいときは [reference/instruction-fields.md](reference/instruction-fields.md) を見ます。

ある命令の仕様を確認したいときは [specs/](specs/) を見ます。

テストコードや作成ツールが何をしているか確認したいときは [test-code-explanations/](test-code-explanations/) を見ます。

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
| `SYSCALL 2` | host標準入力から1 byte読み、`R0`へ入れる |
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

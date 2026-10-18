## Day 33: 入力バッファを読むサブルーチン

```text
名前: word-command-subroutine.bin
分類: sample program
目的: 1行入力をCALLI/RETで呼び出せる処理に分け、help / quit の判定へ戻る
命令長: 4 byte固定
使う命令: MOVI, SYSCALL 1/2, LDB, STB, INC, CMP, JUMP, JZ, JNZ, CALLI, RET, HALT
読むレジスタ: R0, R1, R2
書くレジスタ: R0, R1, R2
読むメモリ: memory[0x500]からの入力バッファ
書くメモリ: memory[0x500]から最大63 byteと終端0 byte、CALLI/RET用スタック
PCの変化: fetchで+4、分岐命令とCALLI/RETで移動
条件フラグの変化: CMPでzero flagを更新
成功条件: help / quit / その他の出力がDay 32と一致する
```

## 呼び出しの約束

メイン処理はプロンプトを表示してから `CALLI read_line` を実行する。`CALLI` は次の命令のPCをスタックに積み、`read_line` へ移動する。ルーチン末尾の `RET` はそのPCを取り出すため、メイン処理は `help` の比較から再開する。

```text
main:
  MOVI R0, 0x300
  SYSCALL 1
  CALLI read_line
  compare_help: ...

read_line:
  R1 = 0x500
  LF、CR、または63 byte到達まで入力
  memory[R1] = 0
  RET
```

`R0`、`R1`、`R2` はルーチン内で変更する。呼び出し側は戻り後にこれらの値を引き継ぐ前提を置かず、`memory[0x500]` の0終端文字列だけを使う。`CALLI` / `RET` のスタックはVMの既存機能を使う。新しい命令やsyscallは追加しない。

## 境界

入力は最大63 byte。64 byte目を読む前に、`memory[0x53F]` に終端0を置く。LFまたはCRで終わった場合は、その時点の `R1` が指す位置に0を置く。入力が途中でEOFになった場合は既存の `SYSCALL 2` の仕様どおり `input EOF` でVMが停止し、`RET` は実行しない。

命令列は `0x300` より前に収め、表示文字列は `0x300`、`0x340`、`0x3A0`、`0x3E0` に配置する。入力バッファは `0x500` から始める。

## 確認

```sh
make programs/word-command-subroutine.bin
make test-word-command-subroutine
```

出力は順に `Commands:`、`Goodbye from Handmade VM`、`Unknown command` となり、それぞれ `CPU halted.` で終わる。`helpful`、空行、63 byteの入力も確認する。

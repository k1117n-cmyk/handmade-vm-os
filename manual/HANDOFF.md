# 作業再開メモ

このファイルは、次回以降に作業を再開するための短い引き継ぎ書です。

過去の作業ログは [WORKLOG.md](WORKLOG.md) に移しました。

## 現在地

Day 25 `echo-char` サンプルまで完了しています。

現在できること:

```text
notes/vm.c
  引数なしなら内蔵テストプログラムを実行する
  引数ありなら外部バイナリを memory[0] から読み込んで実行する
  SYSCALL 2 でhost標準入力から1 byte読んでR0へ入れる

programs/hello.asm
  MOVI R0, 65
  SYSCALL 0
  HALT

programs/echo-char.asm
  SYSCALL 2
  SYSCALL 0
  HALT

tools/small-asm.c
  MOVI Rn, imm / SYSCALL imm / HALT だけを .bin へ変換する
```

## 次回候補

次は候補Aを優先します。

```text
候補A: 1文字入力の次としてプロンプト表示つきechoを作る
候補B: small assembler が扱える命令を1つ増やす
```

完了した仕様カードと個別テスト:

```text
manual/specs/024-syscall-read-char.md
notes/024-syscall-read-char-test.c
manual/test-code-explanations/024-syscall-read-char-test.md
```

完了した最小echoプログラム:

```text
programs/echo-char.asm
programs/echo-char.bin
manual/specs/025-echo-char.md
manual/test-code-explanations/025-echo-char.md
```

次の最初の成功条件:

```text
> を表示する
1文字読む
読んだ文字を表示する
CPU halted.
```

最小echoプログラムの確認:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待出力:

```text
A
CPU halted.
```

いきなり行編集やOS風コマンドループへ進まず、まずは1文字入力だけを小さく確認します。

## 作業方針

新しい命令やVM機能を追加するときは、次の流れを守ります。

```text
1. 仕様カードを書く
2. notes/NNN-name-test.c で部品として小さく確認する
3. 問題なければ notes/vm.c へ統合する
4. programs/*.bin をVMから起動して確認する
5. README と manual の索引を更新する
```

詳しい学習フローは [../HANDWRITING_GUIDE.md](../HANDWRITING_GUIDE.md) を見ます。

作業終わりには [DOC_UPDATE_CHECKLIST.md](DOC_UPDATE_CHECKLIST.md) を見て、周辺ファイルの更新漏れを確認します。

命令番号やfieldは [instruction-types.md](instruction-types.md) と [reference/instruction-fields.md](reference/instruction-fields.md) を基準にします。

## 再開時の確認

まず作業ツリーを確認します。

```sh
git status --short
```

既存の hello バイナリを確認します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待出力:

```text
A
CPU halted.
```

small assembler から生成したバイナリも確認します。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm /tmp/hello-small-asm.bin
/tmp/handmade-vm /tmp/hello-small-asm.bin
```

期待出力:

```text
A
CPU halted.
```

Day 24の個別テストを確認する場合は、標準入力をpipeします。

```sh
cc notes/024-syscall-read-char-test.c -o /tmp/024-syscall-read-char-test
printf A | /tmp/024-syscall-read-char-test
```

`tools/write-hello-bin.c` に触る場合は、次も確認します。

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
xxd programs/hello.bin
/tmp/handmade-vm programs/hello.bin
```

## 注意

今回の企画では、既存教材を組み合わせて完成品を作るのではなく、自作CPU、自作VM、自作OS、自作シェル、自作エディターを一から手書きで組み上げます。

既存教材は、迷ったときや実装後に照合する参考資料として扱います。

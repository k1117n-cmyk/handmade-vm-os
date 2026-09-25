# 作業再開メモ

このファイルは、次回以降に作業を再開するための短い引き継ぎ書です。

過去の作業ログは [WORKLOG.md](WORKLOG.md) に移しました。

## 現在地

Day 28 `command-loop` サンプルまで完了しています。

現在できること:

```text
notes/vm.c
  引数なしなら内蔵テストプログラムを実行する
  引数ありなら外部バイナリを memory[0] から読み込んで実行する
  SYSCALL 2 でhost標準入力から1 byte読んでR0へ入れる
  SYSCALL 3 でR0の下位8bitを改行なしで表示する

programs/hello.asm
  MOVI R0, 65
  SYSCALL 0
  HALT

programs/echo-char.asm
  SYSCALL 2
  SYSCALL 0
  HALT

programs/prompt-echo.asm
  MOVI R0, 62
  SYSCALL 0
  SYSCALL 2
  SYSCALL 0
  HALT

programs/one-char-command.asm
  > を表示して1文字読み、h / q / その他で分岐する
  h なら H を表示する
  q ならそのまま HALT する
  その他なら ? を表示する

programs/command-loop.asm
  > を改行なしで表示して1文字読み、h / q / Enter / space / その他で分岐する
  h なら H を表示してプロンプトへ戻る
  q なら HALT する
  Enter や space なら何も表示せず次の入力を待つ
  その他なら ? を表示してプロンプトへ戻る

tools/small-asm.c
  MOVI Rn, imm / SYSCALL imm / HALT / CMP / JUMP / JZ / JNZ を .bin へ変換する
```

## 次回候補

次は候補Aを優先します。

```text
候補A: 起動メッセージとhelp表示に向けて文字列表示とデータ配置を整理する
候補B: 入力バッファの入口を作る
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

完了したプロンプト付きechoプログラム:

```text
programs/prompt-echo.asm
programs/prompt-echo.bin
manual/specs/026-prompt-echo.md
manual/test-code-explanations/026-prompt-echo.md
```

完了した1文字コマンド判定プログラム:

```text
programs/one-char-command.asm
programs/one-char-command.bin
manual/specs/027-one-char-command.md
manual/test-code-explanations/027-one-char-command.md
```

完了した1文字コマンドループプログラム:

```text
programs/command-loop.asm
programs/command-loop.bin
manual/specs/028-command-loop.md
manual/test-code-explanations/028-command-loop.md
```

次の最初の成功条件:

```text
起動メッセージを表示する
help用の文字列を表示する
SYSCALL 1 で0終端文字列を表示する
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

prompt-echoプログラムの確認:

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/prompt-echo.bin
```

期待出力:

```text
>
A
CPU halted.
```

one-char-commandプログラムの確認:

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/one-char-command.asm programs/one-char-command.bin
cc notes/vm.c -o /tmp/handmade-vm
printf h | /tmp/handmade-vm programs/one-char-command.bin
printf q | /tmp/handmade-vm programs/one-char-command.bin
printf x | /tmp/handmade-vm programs/one-char-command.bin
```

期待出力:

```text
>
H
CPU halted.

>
CPU halted.

>
?
CPU halted.
```

command-loopプログラムの確認:

```sh
make
./handmade-vm programs/command-loop.bin
```

期待出力:

```text
>h
H
>x
?
>q
CPU halted.
```

自動確認する場合:

```sh
make test
```

いきなり行編集へ進まず、次は起動メッセージやhelp表示に必要な文字列表示を小さく確認します。

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

## articles/ の扱い

`articles/` はブログ用の下書き置き場で、意図的にgit管理外にしています。

`articles/` 配下のファイルを作成・更新しても、git管理に入れるかどうかを毎回確認しません。ユーザーから明示的に依頼された場合だけ、git管理へ入れる方法を案内します。

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

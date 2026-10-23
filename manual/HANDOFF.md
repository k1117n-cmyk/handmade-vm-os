# 作業再開メモ

このファイルは、次回以降に作業を再開するための短い引き継ぎ書です。

過去の作業ログは [WORKLOG.md](WORKLOG.md) に移しました。

## 再開時の段取り（最初に読む）

Day 37・第17回は、実装・原稿作成・検証まで完了済みです。次の開始地点は **Day 38・第18回のsmall-asmラベル対応の仕様決め**です。ラベル名の規則、重複・未定義の扱い、前方参照の解決方法はまだ確定していません。

「このファイルを読んで作業を再開してください」と依頼されたら、次の順に進めます。

1. この開始地点と、ユーザーの最新の指示を確認する。完了済みのDay 37・第17回を今回の作業として扱わない。
2. 作業ツリーと必要な既存動作を確認する。これは再開の準備であり、確認結果の報告だけで作業を終えない。同じセッションで確認済みなら、変更や問題がない限り繰り返さない。
3. Day 38の仕様カードに着手する。固定長命令列のラベル定義と参照、前方参照の解決、重複・未定義エラーを小さく決める。文字列データと配置指示は別の段階とする。
4. 仕様に沿って実装・動作確認を進め、第18回の記事構成・本文へつなげる。必要な判断があれば具体的な論点を示し、単に「次へ進めてよいか」を確認するために止まらない。
5. 記事構成と本文の作成は、既存のファイル形式と書式を参考にする。記事構成のmdファイルには、スラッグ、タグ、ディスクリプション、アイキャッチ用のプロンプトを追記する。
6. ブログ記事の本文は、初めて読む人がつまずかないように書く。専門用語は初出時に具体例とともに説明し、作業の目的と期待する動きを示してから、コードや実装の詳細へ進む。前回までの知識が必要な箇所は、理解に必要な要点をその場で補う。
7. 作業終了時に、実際に完了した範囲と次の着手点をこのファイルへ更新する。

原稿の完成状況と公開状況は別に扱います。完了済み原稿の「未公開」という記録は、原稿作成・検証が未完了という意味ではなく、次の実装へ進む妨げにもなりません。ユーザーから指定がない限り、完了済み原稿の再編集・公開確認を再開作業にしません。

## 現在地

Day 37のsmall-asm命令対応拡張まで完了しています。LDB / STB / INC / CALLI / RETを加え、数値オペランドの12命令に対応しました。角括弧内部の空白は不可、レジスタはR0〜R7、CALLIは20bit即値です。

`make test-small-asm`の59ケースで命令値、不正入力、既存5サンプル、Day 36 writerの命令部分との一致、VM実行を確認。`make run-asm-memory-call`はA / B / CPU halted.を表示します。仕様は`specs/037-small-asm-instructions.md`、解説は`test-code-explanations/037-small-asm-instructions.md`です。

最新原稿は`articles/unix-cafe-handmade-vm-17.txt`、構成案は`articles/2026-10-23-vm-small-asm-instructions-037-outline.md`。WordPress形式の未公開下書きで、構成案にスラッグ・タグ・ディスクリプション・アイキャッチ用プロンプトを記載しています。VM本体とecho writerは変更していません。

第17回は読者レビューを受けて説明を改稿済みです。writer・small-asm・VMの役割を冒頭に補い、サンプルの実行と動作説明をCの変換処理より前へ移しました。レジスタ、角括弧、文字コード、PC・SP、ビット演算を具体例で説明し、構成案も本文の順序に合わせています。次の着手点は引き続きDay 38・第18回です。

以下は前回までの完了内容です。

Day 36 `echo-command.bin` まで完了しています。

echoに完全一致するとText>で次の1行を読み、文字列とLFを表示してCommand>へ戻ります。コマンドと本文は0x500のバッファを共用します。本文のhelp / quitは文字列として表示します。入力は最大63 byte、CR無視、LFまで消費。途中EOFでは未完の本文を表示せず停止します。

確認: `make test-echo-command`（Python 3、54ケース）。仕様は `specs/036-echo-command.md`、解説は `test-code-explanations/036-echo-command.md`。実行は `make run-echo-command`。VM本体とDay 35の実装は変更していません。

第16回の原稿は `articles/unix-cafe-handmade-vm-16.txt`（WordPressブロック形式の未公開下書き）。構成案は `articles/2026-10-22-vm-echo-command-036-outline.md`。本文の出力例とブロック対応を確認済みです。

以下は前回までの完了内容です。

Day 35ではhelp / unknownの後にCommand>へ戻り、quitで終了します。入力は最大63 byteを保存し、超過分はLFまで読み捨てます。CRは無視（CRLF対応、CR単独では確定しない）。途中EOFは既存syscallで停止します。

確認: `make test-word-command-loop`（Python 3、26ケースの自動照合）。仕様は `specs/035-word-command-loop.md`、解説は `test-code-explanations/035-word-command-loop.md`。実行は `make run-word-command-loop`。

第15回のブログ原稿は `articles/unix-cafe-handmade-vm-15.txt`（Day 35、複数文字コマンドのループ）。前回の記事は `articles/unix-cafe-handmade-vm-14.txt`（Day 34、文字列比較の共通サブルーチン化）。記事15はWordPressブロック形式の下書きで、未公開です。

Day 34の記事構成案は `articles/2026-10-19-vm-word-command-subroutine-034-outline.md`。記事14の本文に合わせて構成を補完済みです。

Day 35の記事構成案は `articles/2026-10-20-vm-word-command-loop-035-outline.md`。記事15の本文を作成済みです。記事中の4つの出力例をVM実行と照合し、WordPressブロックの対応を確認しています。構成は手動実行、JUMPによる反復、63 byte上限後の読み捨て、CRLF、終端0、自動テストの順です。

Day 34では `tools/write-word-command-compare-bin.c` を追加し、入力と文字列比較をそれぞれCALLI/RETで呼びます。R1/R2に文字列の先頭を渡し、R0は一致なら1、不一致なら0。R0〜R4とzero flagは変更されます。

確認: `make test-word-command-compare`。仕様は `specs/034-word-command-compare.md`、解説は `test-code-explanations/034-word-command-compare.md`。

現在できること:

```text
notes/vm.c
  引数なしならhost側monitor modeを起動する
  引数ありなら外部バイナリを memory[0] から読み込んで実行する
  --self-test なら内蔵テストプログラムを実行する
  monitor modeでは Welcome to Handmade VM と > を表示する
  monitor modeでは入力された .bin を新しいVMへ読み込んで実行する
  monitor modeでは quit / q / exit で終了する
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

programs/boot-message.bin
  命令列の後ろに0終端文字列を配置する
  起動時に Welcome to Handmade VM を表示する
  h なら help用文字列を表示してプロンプトへ戻る
  q なら終了メッセージを表示して HALT する
  tools/write-boot-message-bin.c で生成する

programs/line-buffer.bin
  Type a word, then Enter と > を表示する
  Enterまでの入力を memory[0x180] から1 byteずつ保存する
  最大63 byteまで保存し、最後に0 byteを書いて0終端文字列にする
  入力後に You typed: と保存した文字列を表示する
  tools/write-line-buffer-bin.c で生成する

programs/word-command.bin
  Command> を表示する
  Enterまでの入力を memory[0x500] から1 byteずつ保存する
  入力バッファを help / quit と1 byteずつ比較する
  help なら help用文字列を表示して HALT する
  quit なら終了メッセージを表示して HALT する
  その他なら Unknown command を表示して HALT する
  tools/write-word-command-bin.c で生成する

programs/word-command-subroutine.bin
  入力バッファ処理を CALLI / RET で呼ぶサブルーチンに分ける
  戻った後の help / quit / unknown 判定は Day 32 と同じ
  tools/write-word-command-subroutine-bin.c で生成する

tools/small-asm.c
  MOVI / SYSCALL / HALT / CMP / JUMP / JZ / JNZ / LDB / STB / INC / CALLI / RET を .bin へ変換する
```

## 次の作業: Day 38・第18回

ブログ全体の順序は `articles/ROADMAP.md` に整理済みです。Day 37・第17回の命令対応拡張は完了済みです。

次に着手する作業は、small-asmのラベル対応の仕様カードです。固定長の命令列に絞り、数値指定との共存と前方参照を扱います。文字列データ・引数分割・行編集は後の段階です。

monitor modeはhost側の起動ランチャーであり、まだVM上のOSではありません。VM上のプログラムからhostファイルを開く仕組みは、今後の仮想ディスクやファイル読み込みsyscallのテーマとして扱います。

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

完了した起動メッセージ表示プログラム:

```text
programs/boot-message.bin
tools/write-boot-message-bin.c
manual/specs/029-boot-message.md
manual/test-code-explanations/029-boot-message.md
```

完了した入力バッファ入口プログラム:

```text
programs/line-buffer.bin
tools/write-line-buffer-bin.c
manual/specs/030-line-buffer.md
manual/test-code-explanations/030-line-buffer.md
```

完了したmonitor mode:

```text
notes/vm.c
manual/specs/031-monitor-mode.md
manual/test-code-explanations/031-monitor-mode.md
```

完了した複数文字コマンド判定プログラム:

```text
programs/word-command.bin
tools/write-word-command-bin.c
manual/specs/032-word-command.md
manual/test-code-explanations/032-word-command.md
```

完了した入力サブルーチン版:

```text
programs/word-command-subroutine.bin
tools/write-word-command-subroutine-bin.c
manual/specs/033-word-command-subroutine.md
manual/test-code-explanations/033-word-command-subroutine.md
```

完了した複数文字コマンドループ:

```text
programs/word-command-loop.bin
tools/write-word-command-loop-bin.c
tools/test-word-command-loop.py
manual/specs/035-word-command-loop.md
manual/test-code-explanations/035-word-command-loop.md
```

完了したechoコマンド:

```text
programs/echo-command.bin
tools/write-echo-command-bin.c
tools/test-echo-command.py
manual/specs/036-echo-command.md
manual/test-code-explanations/036-echo-command.md
```

次の最初の成功条件: ラベル定義と参照の規則を仕様カードで決め、数値版と同じbyte列を生成する。前方参照・重複・未定義も確認する。

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

boot-messageプログラムの確認:

```sh
make run-boot-message
```

期待出力:

```text
Welcome to Handmade VM
>h
Commands:
h: help
q: quit
>>q
Goodbye from Handmade VM
CPU halted.
```

line-bufferプログラムの確認:

```sh
make test-line-buffer
```

期待出力:

```text
Type a word, then Enter
>You typed: help
CPU halted.
```

word-commandプログラムの確認:

```sh
make test-word-command
make test-word-command-quit
make test-word-command-unknown
```

期待出力:

```text
Command> Commands:
help: show help
quit: quit
CPU halted.

Command> Goodbye from Handmade VM
CPU halted.

Command> Unknown command
CPU halted.
```

入力サブルーチン版の確認:

```sh
make test-word-command-subroutine
```

`help` / `quit` / その他の出力は `word-command.bin` と同じです。

monitor modeの確認:

```sh
make test-monitor
```

期待出力:

```text
Welcome to Handmade VM
>A
CPU halted.
>Type a word, then Enter
>You typed: help
CPU halted.
>Goodbye from Handmade VM
```

内蔵テストの確認:

```sh
./handmade-vm --self-test
```

期待出力:

```text
A
VM flow complete.
CPU halted.
```

入力・比較サブルーチンを使うループはDay 35で完了しました。Day 36ではechoの追加まで完了しました。Day 37でsmall-asmの5命令対応も完了し、次はラベル対応へ進みます。

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

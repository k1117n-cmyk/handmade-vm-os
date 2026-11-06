# Handmade VM OS

自作CPU、自作VM、自作OS、自作シェル、自作エディターへ進むためのシリーズ用リポジトリです。

完成コードを急いで作るのではなく、1つの命令を小さく分解し、仕様カード、手計算、`fetch`、`decode`、`execute`、Cコードへの置き換えを順番に確認します。

公開記事:

- [【低レイヤ入門】C言語で仮想マシン自作：アセンブリからVM実装への「目線の切り替え」を学ぶ](https://pc-fan.net/handmade-vm-builder-eye/)

## まず読むもの

最初は次の順番で読むと、目的と練習方法が追いやすいです。

1. [ROADMAP.md](ROADMAP.md)
2. [HANDWRITING_GUIDE.md](HANDWRITING_GUIDE.md)
3. [vm-builder-eye-practice.md](vm-builder-eye-practice.md)
4. [manual/README.md](manual/README.md)
5. [manual/instruction-types.md](manual/instruction-types.md)
6. [manual/reference/instruction-fields.md](manual/reference/instruction-fields.md)
7. [WORKBOOK_001_HALT.md](WORKBOOK_001_HALT.md)

`HALT` をかなりやさしく分解した説明は [HALT_FOR_BEGINNERS.md](HALT_FOR_BEGINNERS.md) にあります。

## このリポジトリで扱うこと

- 1MB の byte-addressed memory を持つ小さなVM
- 32bit big-endian instruction fetch
- `R0`-`R7`, `PC`, `SP`, `zero_flag` を使う最小CPU状態
- `HALT`, `RET`, `MOV`, `INC`, `DEC`, `ADD`, `SUB`, `CMP`, `MOVI`, `SYSCALL`, `JUMP`, `CALLI`, `JZ`, `JNZ`, `LDB`, `STB`, `LDDI`, `STDI`, `PUSH`, `POP` などの命令を1つずつ実装する練習
- アセンブリ表記、命令field、Cコードの対応づけ

最終的には次の形を目指します。

```text
host macOS/Linux
  -> 自作VM本体 C
  -> 自作CPU ISA
  -> 自作OS
  -> 自作shell
  -> 自作editor
```

## ディレクトリ構成

```text
manual/
  README.md
  HANDOFF.md
  WORKLOG.md
  instruction-types.md
  specs/
  reference/
  test-code-explanations/

notes/
  000-machine-test.c
  001-halt-test.c
  ...
  009-stb-test.c
  010-lddi-test.c
  011-stdi-test.c
  012-inc-dec-test.c
  013-mov-test.c
  014-push-test.c
  015-pop-test.c
  016-add-sub-test.c
  017-cmp-test.c
  018-jump-jz-test.c
  019-jnz-test.c
  020-calli-ret-test.c
  021-binary-loader-test.c
  024-syscall-read-char-test.c
  vm.c

programs/
  README.md
  hello.asm
  hello.bin
  echo-char.asm
  echo-char.bin
  prompt-echo.asm
  prompt-echo.bin
  one-char-command.asm
  one-char-command.bin
  command-loop.asm
  command-loop.bin
  line-buffer.bin
  word-command.bin
  word-command-subroutine.bin
  word-command-compare.bin
  word-command-loop.bin
  echo-command.bin
  asm-memory-call.asm
  asm-memory-call.bin
  asm-label-call.asm
  asm-label-call.bin

tools/
  small-asm.c
  write-hello-bin.c
  write-boot-message-bin.c
  write-line-buffer-bin.c
  write-word-command-bin.c
  write-word-command-subroutine-bin.c
  write-word-command-compare-bin.c
  write-word-command-loop-bin.c
  write-echo-command-bin.c
```

`manual/` は仕様、補助資料、テストコード解説をまとめたマニュアルです。

`notes/` はCコード置き場です。各 `*-test.c` は、その日の命令やVM部品を小さく確認するためのテストコードです。`vm.c` は現在の統合VMです。

Day 22 と Day 23 はVM命令の個別テストではなく作成ツールの追加なので、`notes/022-...` と `notes/023-...` はありません。対応するコードは `tools/write-hello-bin.c` と `tools/small-asm.c` です。

Day 25 はサンプルプログラムの追加なので、`notes/025-...` はありません。対応するコードは `programs/echo-char.asm` と `programs/echo-char.bin` です。

Day 26 もサンプルプログラムの追加なので、`notes/026-...` はありません。対応するコードは `programs/prompt-echo.asm` と `programs/prompt-echo.bin` です。

Day 27 と Day 28 もサンプルプログラムの追加なので、`notes/027-...` と `notes/028-...` はありません。対応するコードは `programs/one-char-command.asm` / `programs/one-char-command.bin` と `programs/command-loop.asm` / `programs/command-loop.bin` です。

`notes/vm.c` は、外部バイナリを読み込んで実行できます。

```text
./vm program.bin
```

引数なしで起動した場合は、host側のmonitor modeに入ります。monitorは、入力された外部バイナリのパスを読み、その `.bin` を新しいVMへ読み込んで実行します。

```text
./handmade-vm
Welcome to Handmade VM
>programs/hello.bin
A
CPU halted.
>quit
Goodbye from Handmade VM
```

これはまだVM上のOSではありません。host側のCコードがファイル名を読み、外部バイナリを開いて `memory[0]` へ読み込む入口です。

最初のサンプルとして、`MOVI R0, 65`, `SYSCALL 0`, `HALT` だけの小さな `programs/hello.asm` と `programs/hello.bin` を置いています。

`programs/hello.bin` は、次の小さな作成ツールで再生成できます。

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
```

また、最小限のアセンブリ表記から生成することもできます。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm programs/hello.bin
```

作業を再開するときは [manual/HANDOFF.md](manual/HANDOFF.md)、過去の作業ログを確認するときは [manual/WORKLOG.md](manual/WORKLOG.md) を見ます。

OS風プログラムに必要な入力系として、`SYSCALL 2` でhost標準入力から1 byte読み、`R0`へ入れられるようにしています。仕様カードは [manual/specs/024-syscall-read-char.md](manual/specs/024-syscall-read-char.md) にあります。

`SYSCALL 2` の最小サンプルとして、標準入力から読んだ1文字をそのまま表示する `programs/echo-char.asm` と `programs/echo-char.bin` も置いています。

また、入力前に `>` を表示してから1文字echoする `programs/prompt-echo.asm` と `programs/prompt-echo.bin` も置いています。

さらに、入力された1文字を `h`, `q`, その他に分ける `programs/one-char-command.asm` と、`q` が入力されるまでプロンプトへ戻る `programs/command-loop.asm` も置いています。

起動メッセージとhelp表示を持つ `programs/boot-message.bin`、Enterまでの入力をVM内メモリに貯める `programs/line-buffer.bin`、入力バッファを `help` / `quit` と比較する `programs/word-command.bin` も置いています。Day 33 の `programs/word-command-subroutine.bin` は、入力部分を `CALLI` / `RET` で呼び出す版です。これらは、命令列と文字列データを専用writerで生成するサンプルです。

## 試し方

現在の最小VMは `notes/vm.c` です。

ルートディレクトリに `handmade-vm` を作る場合は、次のように実行します。

```sh
make
```

これで `./handmade-vm` と、`programs/*.asm` から生成する `programs/*.bin` が用意されます。

プロンプトへ戻る1文字コマンドループは、手でVMを起動して確認できます。

```sh
./handmade-vm programs/command-loop.bin
```

起動後、キーボードから `h`、`x`、`q` を順番に入力します。各文字のあとに Enter を押してかまいません。

```text
>h
H
>x
?
>q
CPU halted.
```

同じ確認を自動化する場合は、次のコマンドも使えます。

```sh
make test
```

引数なしで起動すると、monitor modeに入ります。

```sh
./handmade-vm
```

手動では、プロンプトに実行したい外部バイナリのパスを入力します。

```text
Welcome to Handmade VM
>programs/hello.bin
A
CPU halted.
>quit
Goodbye from Handmade VM
```

自動確認する場合は、次のターゲットを使えます。

```sh
make test-monitor
```

期待する出力:

```text
Welcome to Handmade VM
>A
CPU halted.
>Type a word, then Enter
>You typed: help
CPU halted.
>Goodbye from Handmade VM
```

パイプで入力した場合は端末側の入力echoがないため、monitorの `>` と実行したプログラムの出力が同じ行に続いて見えます。

内蔵テストプログラムを実行する場合は、`--self-test` を渡します。

```sh
./handmade-vm --self-test
```

期待する出力:

```text
A
VM flow complete.
CPU halted.
```

外部バイナリを読み込む場合は、引数にファイル名を渡します。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待する出力:

```text
A
CPU halted.
```

入力系の最小サンプルも実行できます。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/echo-char.bin
```

期待する出力:

```text
A
CPU halted.
```

プロンプト付きの最小サンプルも実行できます。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf A | /tmp/handmade-vm programs/prompt-echo.bin
```

期待する出力:

```text
>
A
CPU halted.
```

プロンプトへ戻る1文字コマンドループも実行できます。

```sh
cc notes/vm.c -o /tmp/handmade-vm
printf hxq | /tmp/handmade-vm programs/command-loop.bin
```

期待する出力:

```text
>H
>?
>CPU halted.
```

Enterまでの入力をVM内メモリに保存するサンプルも実行できます。

```sh
make test-line-buffer
```

期待する出力:

```text
Type a word, then Enter
>You typed: help
CPU halted.
```

入力バッファを複数文字コマンドとして比較するサンプルも実行できます。

```sh
make test-word-command
make test-word-command-quit
make test-word-command-unknown
```

期待する出力:

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

入力処理をサブルーチンに分けた版は次で確認できます。`help` / `quit` / その他の出力は上と同じです。

```sh
make test-word-command-subroutine
```

個別の練習コードも同じようにコンパイルして実行できます。

```sh
cc notes/008-ldb-test.c -o /tmp/008-ldb-test
/tmp/008-ldb-test
```

```sh
cc notes/009-stb-test.c -o /tmp/009-stb-test
/tmp/009-stb-test
```

```sh
cc notes/011-stdi-test.c -o /tmp/011-stdi-test
/tmp/011-stdi-test
```

```sh
cc notes/012-inc-dec-test.c -o /tmp/012-inc-dec-test
/tmp/012-inc-dec-test
```

```sh
cc notes/013-mov-test.c -o /tmp/013-mov-test
/tmp/013-mov-test
```

```sh
cc notes/014-push-test.c -o /tmp/014-push-test
/tmp/014-push-test
```

```sh
cc notes/015-pop-test.c -o /tmp/015-pop-test
/tmp/015-pop-test
```

```sh
cc notes/016-add-sub-test.c -o /tmp/016-add-sub-test
/tmp/016-add-sub-test
```

```sh
cc notes/017-cmp-test.c -o /tmp/017-cmp-test
/tmp/017-cmp-test
```

```sh
cc notes/018-jump-jz-test.c -o /tmp/018-jump-jz-test
/tmp/018-jump-jz-test
```

```sh
cc notes/019-jnz-test.c -o /tmp/019-jnz-test
/tmp/019-jnz-test
```

```sh
cc notes/020-calli-ret-test.c -o /tmp/020-calli-ret-test
/tmp/020-calli-ret-test
```

```sh
cc notes/021-binary-loader-test.c -o /tmp/021-binary-loader-test
/tmp/021-binary-loader-test
```

```sh
cc notes/024-syscall-read-char-test.c -o /tmp/024-syscall-read-char-test
printf A | /tmp/024-syscall-read-char-test
```

## 学習の型

新しい命令を見たら、次の3つの目線で分けます。

```text
1. アセンブリを書く人の目線
2. 命令をdecodeするVMの目線
3. 実際に値を動かすCコードの目線
```

例:

```asm
LDB R0, [R1]
```

```text
Assembly        C
R0              regs[0]
R1              regs[1]
[R1]            memory[regs[1]]
LDB R0, [R1]    regs[0] = memory[regs[1]]
```

この置き換えを手で追う練習用の短いガイドが [vm-builder-eye-practice.md](vm-builder-eye-practice.md) です。

命令タイプと命令一覧は [manual/instruction-types.md](manual/instruction-types.md)、各命令の仕様は [manual/specs/](manual/specs/) にあります。

## ライセンスと出典

コードと文書は [MIT License](LICENSE) で公開します。

参考にしている教材や記事は [ATTRIBUTION.md](ATTRIBUTION.md) にまとめています。

GitHub公開前の確認手順は [PUBLISHING.md](PUBLISHING.md) にまとめています。

### Day 34: 文字列比較を共通サブルーチンへ

`programs/word-command-compare.bin` は、Day 33の入力サブルーチンに加え、`help` / `quit` の比較を同じ `string_equal` で行います。R1・R2で文字列を渡し、R0で一致なら1、不一致なら0を受け取ります。

```sh
make run-word-command-compare
# help / quit / hello の3分岐を順に確認
make test-word-command-compare
```

生成元は `tools/write-word-command-compare-bin.c`。入力なしで起動するmonitorからも `programs/word-command-compare.bin` を指定できます。[比較ルーチンの解説](manual/test-code-explanations/034-word-command-compare.md)で命令列を追えます。

### Day 35: 複数文字コマンドを繰り返す

`programs/word-command-loop.bin` はhelp / unknownの表示後に再入力し、quitで終了します。最大63 byteを保存し、超過分もLFまで読み捨てます。CRは無視するためCRLFにも対応します（CR単独では確定しません）。

```sh
make run-word-command-loop
# help、hello、quit を順に入力
make test-word-command-loop
```

writerは `tools/write-word-command-loop-bin.c`。テストはPython 3で26ケースの出力を自動照合します。[仕様](manual/specs/035-word-command-loop.md)と[命令列の解説](manual/test-code-explanations/035-word-command-loop.md)を参照してください。

## Day 36: echoコマンド

`programs/echo-command.bin` はhelp / quitのループにechoを追加します。`echo`と完全一致すると`Text> `で次の1行を読み、表示して`Command> `へ戻ります。本文のhelpやquitは文字として表示します。`echo hello`の引数形式はまだ扱いません。

```sh
make run-echo-command
make test-echo-command
```

入力はコマンド・本文とも最大63 byte、CRを無視しLFで確定、超過分もLFまで消費します。途中EOFでは未完の行を実行・表示せず停止します。本文が空なら改行だけを表示します。

writerは `tools/write-echo-command-bin.c`、自動テストは `tools/test-echo-command.py`（Python 3、54ケース）です。[仕様](manual/specs/036-echo-command.md)と[解説](manual/test-code-explanations/036-echo-command.md)を参照してください。

## Day 37: small-asmの命令対応拡張

`tools/small-asm.c`に`LDB Rd, [Rs]`、`STB [Rd], Rs`、`INC Rd`、`CALLI imm`、`RET`を追加しました。既存7命令と合わせて12命令に対応します。呼び出し先は数値で指定し、レジスタはR0〜R7です。角括弧の内部に空白は入れません。Day 37時点では数値指定のみです。ラベルは下記のDay 38で追加し、文字列データは下記のDay 39で扱います。

```sh
make run-asm-memory-call
make test-small-asm
```

サンプルはAとBをメモリへ保存し、同じサブルーチンを2回呼んで1文字ずつ表示します。実行結果は`A`、`B`、`CPU halted.`の3行です。Day 37時点ではPython 3による59ケースのテストで、命令値、不正入力、既存サンプルとDay 36 writerとの一致、VM実行を確認します。[仕様](manual/specs/037-small-asm-instructions.md)と[解説](manual/test-code-explanations/037-small-asm-instructions.md)を参照してください。

## Day 38: small-asmのラベル対応

分岐とCALLIの宛先を `CALLI print_byte` のように名前で指定できます。`print_byte:` はその位置のアドレスを定義し、出力byteを増やしません。前方参照にも対応し、数値指定も引き続き使えます。名前はASCII英字か `_` で始まる英数字・`_` の最大63文字で、大小文字を区別しません。Day 38時点ではMOVIとSYSCALLは数値のみです（MOVIのラベル参照はDay 39で追加）。

```sh
make run-asm-label-call
make test-small-asm test-small-asm-labels
```

数値版と同じ56 byteを生成し、A / B / CPU halted.を表示します。既存57ケースとラベル44ケースを自動照合します（旧59ケースのうちラベル関係2件はラベル用テストへ移動）。重複・未定義・上限エラーも検査し、入力エラー時は指定出力を保持します。[仕様](manual/specs/038-small-asm-labels.md)と[解説](manual/test-code-explanations/038-small-asm-labels.md)を参照してください。


## Day 39: small-asmの文字列と配置

`.string "Hello, VM!\n"` でASCIIの0終端文字列を置き、`.align 4` で次の命令位置を4の倍数へ揃えます。`MOVI R0, message` でデータのラベルも参照できます。

```sh
make run-asm-string
make test-small-asm-strings
```

`programs/asm-string.asm` から28 byteの `.bin` を生成し、Hello, VM! / CPU halted.を表示します。文字列用52ケースを自動照合。VM本体とwriterは変更していません。[仕様](manual/specs/039-small-asm-strings.md)と[解説](manual/test-code-explanations/039-small-asm-strings.md)を参照してください。

## Day 40: echoを.asmへ移す

`programs/asm-echo-command.asm`から、70命令と8文字列の421 byteを生成します。文字列は命令の直後へ並べ、入力バッファは0x500を維持します。絶対配置指示の追加は不要でした。

```sh
make run-asm-echo-command
make test-asm-echo-command
```

echoの後にhello、quitを各行で入力すると、本文を表示してコマンド入力へ戻り、終了します。テストは配置照合と既存writer版・asm版それぞれ54ケースです。[仕様](manual/specs/040-asm-echo-command.md)と[解説](manual/test-code-explanations/040-asm-echo-command.md)を参照してください。

## Day 41: コマンド名と本文を分ける

`programs/command-arguments.asm`から600 byteのバイナリを生成します。`echo hello`は同じ行の本文を表示し、`echo`単独は従来どおりText>で次の行を読みます。先頭・区切りの半角空白を読み飛ばし、本文中と末尾の空白は保持します。`echo `は空本文です。help / quitへの空白以外の引数は拒否します。

```sh
make run-command-arguments
make test-command-split test-command-arguments
```

入力はコマンド名・空白も含め最大63 byteです。分割部品14ケース、入出力74ケースを確認します。引用符の解釈・複数引数・行編集は今後の候補です。[仕様](manual/specs/041-command-arguments.md)と[解説](manual/test-code-explanations/041-command-arguments.md)を参照してください。


## Day 42: メモリ上の1行を保持・表示・置換する

`programs/line-editor.asm` / `.bin`は、`set hello`で内容を置き、`show`で表示します。
入力0x500とは別の編集バッファ0x600に最大63 byteと終端0を保持し、echoやhelpの後も内容が残ります。
set単独はText>で次の1行、`set `（区切りだけ）は空内容への置換です。quitでは保存せず終了し、再起動すると空になります。
同じ行の本文は最大59 byte、次の行なら最大63 byteです。

```sh
make run-line-editor
make test-line-copy test-line-editor
```

生成物は147命令・10文字列・828 byte。部品8・入出力75ケースを照合します。
[仕様](manual/specs/042-line-editor.md)と[解説](manual/test-code-explanations/042-line-editor.md)を参照してください。


## Day 43: 1行をファイルへ保存・読み込みする

`programs/line-storage.asm` / `.bin`で、set / showにsave / loadを追加しました。
VMプロセスの作業ディレクトリにある固定名`line.txt`へ、本文0〜63 byteだけを保存します。
終端NUL・改行は書きません。既存ファイルは直接上書きし、保存途中の失敗では旧ファイルを保持できない場合があります。
loadはファイル全体を検証してから置換し、失敗時は編集内容を保持します。
起動時は空で、自動読み込み・自動保存は行いません。

```sh
make run-line-storage
make test-line-storage-parts test-line-storage
```

入力例は `set hello` → `save` → `quit`。再起動後に`load` → `show`でhelloを表示します。
save / loadは成功時にSaved. / Loaded.、失敗時にSave failed. / Load failed.を表示して入力へ戻ります。
178命令・17文字列・1089 byte。部品38・VM経由38・入出力142ケースを確認します。
[仕様](manual/specs/043-line-storage.md)と[解説](manual/test-code-explanations/043-line-storage.md)を参照してください。

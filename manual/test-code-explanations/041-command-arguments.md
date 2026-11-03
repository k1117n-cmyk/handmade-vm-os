# Day 41: コマンド名と本文の分割を確かめる

## 手で動かす

```sh
make run-command-arguments
```

Command>へ`echo hello`、`echo`、Text>へ`hello`、Command>へ`quit`を入力する。先頭のechoは同じ行のhelloを表示し、echo単独だけが次の行を待つ。

```text
Command> echo hello
hello
Command> echo
Text> hello
hello
Command> quit
Goodbye from Handmade VM
CPU halted.
```

上の表示例には端末が表示する入力文字を含む。pipe実行では入力文字自体は表示されない。`echo `（末尾に半角空白）なら空本文のLFを表示し、Text>には進まない。

## 1行を2つの文字列にする

入力バッファは0x500〜0x53F。`echo hello`の区切りは0x504にあり、split_lineがそこを0へ置き換える。R6は0x505（本文の先頭）、R7は1（区切りあり）になる。echo単独ではR6が0x504の終端0、R7が0になる。

先頭空白と連続する区切りだけを読み飛ばし、本文中・末尾の空白は保持する。引数の配列やコピー用バッファは用意しない。R6・R7を比較処理が壊さないことで、判定後も本文の位置と区切りの有無を使える。

compare_commandは毎回R1を0x500に戻し、先頭空白を読み飛ばしてから従来のstring_equalへ進む。small-asmにはMOV構文がないため、ポインタのレジスタ間コピーを追加せず、本文は`LDB R0, [R6]`とSYSCALL 3で表示する。

help / quitは本文が空の場合だけ比較する。echo単独では既存read_lineでバッファを再利用して次の1行を読む。

ヘルプのechoの案内は、旧版の`echo: read and print a line`から`echo [text]: print text or read a line`へ変更している。本文を同じ行へ指定する方法と省略時に次の行を読む方法を示す文言で、仕様カードへの追記後、ユーザー承認を得て確定した。

## 部品テストから統合テストへ

```sh
make test-command-split
make test-command-arguments
```

`tools/test-command-split.py`はnotesのCテストではなく、実ソースからsplit_lineを取り出して一時的なVMプログラムを組み立てるPythonテスト。14個の固定ベクトルで、バッファ全体の期待byte、R6の位置、R7の値を検査する。空白、空本文、タブ、NUL、63 byte境界を含む。テスト用命令列は0x600から置き、入力バッファと重ならないようにする。

`tools/test-command-arguments.py`は実際の.binをVMで動かす。73個の直接実行と1個のmonitor実行の計74ケース。終了コード0、標準エラーが空、標準出力全体が期待値と一致することを確認する。先頭・連続・末尾空白、空本文、echo単独、引数付きhelp / quitの拒否、CRLF、途中EOF、NUL、UTF-8、62/63/64/4096 byte、1000回反復を含む。

63 byteはコマンド名や空白も含む。`echo `の5 byteを除くと本文は最大58 byte。上限を超える入力もLFまで消費し、次のコマンドへ混ざらない。保存後の最初のNULまでを処理するため、NUL以後や保存上限以後の空白は区切りとして使わない。

## 配置と結果

生成物は112命令448 byteと8文字列152 byteで600 byte（末尾の次は0x258）。0x500のバッファと重ならない。サイズの上限は専用入出力テストでも確認する。

旧asm版は70命令・421 byte、writer版は1127 byteのまま。新旧では動作と命令列が違うため、Day 41の.binと旧版との全体cmpは要求しない。Day 40の配置照合、writer版・asm版各54ケースは維持する。

今回の確認コマンド:

```sh
make test-command-split test-command-arguments
make test-small-asm test-small-asm-labels test-small-asm-strings
make run-asm-string
make all test test-monitor test-echo-command test-word-command-loop
make test-asm-echo-command
./handmade-vm --self-test
cmp programs/asm-memory-call.bin programs/asm-label-call.bin
```

すべて終了コード0。既存の件数はsmall-asm 57、labels 44、strings 52、echo各54、word-command-loop 26で変化なし。VM本体・small-asm・writerへの変更は不要だった。[仕様カード](../specs/041-command-arguments.md)を参照。

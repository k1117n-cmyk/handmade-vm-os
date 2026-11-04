# Day 42: 入力とは別に1行を保持する

## 手で動かす

```sh
make run-line-editor
```

入力文字を含む端末表示:

```text
Command> set hello
Command> show
hello
Command> set world
Command> show
world
Command> quit
Goodbye from Handmade VM
CPU halted.
```

setは保持内容全体を置換し、追加の完了表示を出さずCommand>へ戻ります。
set単独ならText>で次の行、`set `（区切りだけ）なら空内容へ置換します。
showは保持内容とLFを表示。初期内容は空で、quit時は保存せず終了し、再起動すると空です。

## メモリとレジスタ

入力は0x500〜0x53F、編集は0x600〜0x63F。各63 byteと終端0です。
147命令588 byteと10文字列240 byteは0から828 byte（末尾の次は0x33C）へ連続配置。
入力バッファと重ならないことを入出力テストでも検査します。

同じ行のset helloでは分割後のR6=0x504、R7=1。set単独ならread_line後にR6=0x500とします。
次の行にはsplit_lineを呼ばないので先頭空白も保持します。同じ行の本文は最大59 byte、次の行は最大63 byte。

copy_lineはR6にコピー元を受け、R5を0x600に設定し、LDB→STB→終端判定→両ポインタのINCを繰り返します。
終端0をコピーしてからRETするため、長い内容から短い内容へ置換しても古い末尾が見えません。
空なら0x600へ0だけを書きます。R0・R3・R5・R6とzero flagを変更し、R1・R2・R4・R7を保持。
復帰時のR5・R6は終端0を指します。入力側に63 byte以内の終端0があることは呼び出し側の前提です。

showはR0=0x600でSYSCALL 1を呼び、print_newlineへJUMPしてLFを出します。
help / quit / showは非空引数を拒否。echoは編集バッファに書き込みません。
CALLI / RETの入れ子はなく、SPは初期値0x100000または0xFFFFCで、バッファとは重なりません。

## 部品確認から統合へ

```sh
make test-line-copy test-line-editor
```

`tools/test-line-copy.py`は実ソースのcopy_lineを取り出すPythonテストです。
最初はコピー部品だけを作り、通常・空・63 byte・短縮・空への置換・NUL・本文位置0x504・終端位置0x53Fの8ケースを確認しました。
コピー元64 byte、編集先64 byteと前後の目印、保持レジスタ、復帰時のポインタを固定期待値で照合します。
テスト用命令列は0x800から置き、データバッファを命令で初期化してからCALLIで呼びます。
部品通過後に同じ処理をコマンドループへ組み込みました。

`tools/test-line-editor.py`は74直接実行と1 monitor実行の計75ケースです。
標準出力全体・終了コード0・空の標準エラーを照合します。
保持・表示・置換・空内容・echo後の保持、引数拒否、空白、CRLF、途中EOF、NUL、UTF-8、
0/1/59/62/63/64/4096 byteとLF/CRLF、1000回の操作列反復を含みます。
monitorでは2回起動し、終了後の復帰と、2回目に内容が残らないことを確認します。

上限後はLFまで消費、CRは無視。途中EOFはコピーせず停止します。
NULまでをコピーし、UTF-8もbyte単位なので文字が途中で切れる場合があります。

## 実行した確認

```sh
make test-line-copy test-line-editor
make test-command-split test-command-arguments
make test-small-asm test-small-asm-labels test-small-asm-strings
make run-asm-string
make all test test-monitor test-echo-command test-word-command-loop
make test-asm-echo-command
./handmade-vm --self-test
cmp programs/asm-memory-call.bin programs/asm-label-call.bin
```

すべて終了コード0。新規8 + 75、既存14 + 74、assembler 57 + 44 + 52、echo各54、loop26ケースを確認。
Day 40の配置照合も通過。VMを別途警告付きでビルドし、monitorとhelloも確認しました。
詳細なコマンド・生ログ・開始状態は `wp/codex-completion-report.md` と `wp/codex-day42-evidence.json` に保存（git管理外）。
[仕様カード](../specs/042-line-editor.md)を参照してください。

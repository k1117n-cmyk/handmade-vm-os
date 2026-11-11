# Day 44 別プログラムを実行して親へ戻る

`make run-program-launcher` をリポジトリのルートで実行し、set keep → run → show → quitを各行で入力する。
runは固定の `programs/run-child.bin` を起動する。子はAとCPU halted.を表示し、親はRun finished.を表示する。
showでは親が保持していたkeepが表示される。子の起動失敗・異常停止ではRun failed.を表示して再入力へ戻る。

## ロード部品とVMへの統合

`notes/program-launch.h` の `load_child_program` は、初期化済み子メモリへ固定ファイルを読む。
4 byte以上1 MiB以下を受理する。容量ちょうどなら追加の1 byteを調べ、超過と読み取り失敗を検出する。
読み取りとcloseを確認してから成功とし、失敗した子は実行しない。ロード部品は画面出力を行わない。

まず `notes/044-program-launch-test.c` のロード単体13ケースを確認した。
欠落、0〜3 byte、4 byte、容量の前後、open/read/close・末尾確認の失敗を含む。
stdioの失敗はテスト内だけ置換し、実際のファイルを閉じてからcloseの失敗を返す。
末尾確認の失敗は追加読み取りを通ったことも検査する。

その後 `notes/vm.c` のsyscall 6へ統合した。子VMをmallocで確保して既存のinit_vmで初期化し、
子を示すis_childを設定する。子のsyscall 6はR0へ1を返し、その子の次の命令へ進む。
runはHALTで止まった場合にtrueを返す。EOF・不正命令・範囲外アクセス等の停止はfalseを返す。
子の実行後またはロード失敗後にfreeし、親のR0へ0または1を返す。
親の命令・メモリ・スタックを退避して上書きする操作は不要になる。

```sh
make test-program-launch-parts
```

同じテストを `-DLAUNCH_VM_TEST` 付きでビルドし、ロード13ケースに加えVM経由17ケースを確認する。
初期状態1ケースとsyscall経由16ケースで、親の全メモリ、R0〜R7、SP、zero flag、running、fetch後のPCを照合する。
子による編集領域への書き込み・PUSH・フラグ変更、正常終了、異常停止、EOF、
確保失敗、ロード失敗、子からの起動要求を含む。開いたファイルと確保した子VMの解放数を照合する。

## 親子の入出力と配置

```sh
make test-program-launch
```

`tools/test-program-launch.py` は、新規28ケースと共通の保存・編集テスト142ケースを実行する。
共通の `tools/test-line-storage.py` に任意の `--launcher` を追加し、承認済みのhelp追加行と親のソースを選ぶ。
従来の呼び出しではDay 43の142ケースと固定期待値を引き続き使う。

新規ケースでは成功・失敗、空白・CR・NUL・上限、引数拒否、子の標準入力EOF、
親子の共有入力とline.txt、子の1000回の初期化、monitorでの親再起動を確認する。
終了コード0、stdout全文、空のstderr、ファイルの本文と子binの保持を照合する。
ファイルはすべて一時ディレクトリ内で扱う。

親は191命令764 byteと20文字列434 byte、合計1198 byte。末尾の次0x4AEは入力0x500より前で、編集0x600とも重ならない。
子はMOVI R0, 65 / SYSCALL 0 / HALTの3命令12 byte。親子それぞれ再アセンブルとの全byte一致を検査する。
子のメモリとレジスタは起動ごとに初期化される。標準入力の消費とline.txtの変更はhost資源の状態として共有する。
実行が停止しなければ親は待ち続ける。

Day 43の回帰セット全体、VM自己テスト、既存バイナリのcmp、別ビルドVMと通常のMakefile入口も確認した。
[仕様カード](../specs/044-program-launch.md)、wp/codex-day44-evidence.jsonの生ログとwp/codex-completion-report.mdを参照。

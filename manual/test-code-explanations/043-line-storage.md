# Day 43: 固定名ファイルへ1行を保存する

## 操作と保存形式

```sh
make run-line-storage
```

set hello → save → quitを各行で入力すると、VMプロセスの作業ディレクトリのline.txtへ
`hello`の5 byteを保存します。NUL・LFはファイルへ書きません。再起動時の内容は空で、load → showで復元します。
save / loadの非空引数は拒否し、成功ならSaved. / Loaded.、失敗ならSave failed. / Load failed.を表示します。

保存は直接上書きです。open前の検証失敗は既存ファイルを保持しますが、書き込み開始後に失敗すると、
旧ファイルは失われたり途中の内容が残ったりします。読み込みは成功するまで編集内容を変更しません。
空ファイルは空内容として成功し、64 byte以上、NUL・CR・LFを含むファイルは失敗します。
UTF-8はbyte列として扱い、切り捨て・改行変換・文字の妥当性検査を行いません。

## 部品を確認してから統合する

`notes/line-storage.h`の共通部品に、メモリ先頭、サイズ、64 byte領域のアドレスを渡します。
保存は終端0とCR/LFの検証後にwbで開き、本文の書き込み数とcloseを確認します。
読み込みはhost側の64 byte一時領域に読み、EOF・read/close成功・容量・内容を検証してから本文と終端0をコピーします。
アドレスは`address > memory_size - 64`で比較し、範囲検査時の加算の桁あふれを避けます。

部品単体の38ケースを通過してから、同じ部品をnotes/vm.cへ統合しました。
syscall 4/5はR0のアドレスを受け、R0へ成功0・失敗1を返します。
R1〜R7・zero flag・SPを保持し、PCはfetchによる4 byte前進のまま。ファイルの失敗でもVMは動き続けます。

```sh
make test-line-storage-parts
```

`notes/043-line-storage-test.c`を単体用と`-DSTORAGE_VM_TEST`付きの2通りでビルドします。
後者は実VMのfetch/decode/executeを呼び、38ケースの各呼び出しでレジスタ・フラグ・SP・PC・runningも照合します。
両方ともメモリ全体を固定期待値と比較し、領域外の書き換えやload失敗時の変更を検出します。
0、末尾の有効64 byte、1 byteの範囲超過、最終byte、メモリ外、UINT32_MAXを確認します。
メモリとファイルの内容には空・通常・63/64 byte・大きすぎる内容・NUL/CR/LF・UTF-8と非UTF-8 byteを含めます。

一時ディレクトリ内で実ファイルを使います。open/read/write/close失敗はテスト内だけstdioを置換して注入します。
close失敗の注入でも実ファイルは閉じ、開けたファイルが毎回閉じられたことを確認します。
部分書き込みとclose失敗は失敗を返し、ファイル保持を保証しない仕様に沿って実際の書き込み結果も照合します。
テスト終了時は自分で作ったファイルとディレクトリを除去します。

## 新サンプルの配置と入出力

```sh
make test-line-storage
```

178命令712 byte + 17文字列377 byte = 1089 byte。末尾の次は0x441で、入力0x500〜0x53F、
編集0x600〜0x63Fより手前です。CALLI/RETの入れ子はなく、戻り先スタックは0xFFFFC〜0xFFFFFです。
helpは既存のhelp_textと追加のstorage_help_textを続けて表示し、1つの長すぎるソース行を作りません。

`tools/test-line-storage.py`は入出力142ケースを実行します。Day 42の75ケースを新しいhelpの期待値で継承し、
保存・読み込み67ケースを追加しました。終了コード0・stdout全体・空のstderr・ファイルの有無と全byteを照合します。
再起動を別VMプロセスとmonitorの両方で確認し、初期内容は空、明示loadでだけ復元することを検査します。
入力上限とCRLF、未知コマンドやecho後の保持、EOF/quitで保存しないこと、失敗後の継続、1000回の反復も含みます。
テストの保存先は毎回一時ディレクトリで、リポジトリのline.txtへ書き込みません。

別途再アセンブルしたbinと配布対象binの全byte一致、178命令・17文字列の連続配置、syscall 4/5の命令値、
ファイル末尾が0x500以下であることを照合します。この配置検査は142入出力ケースとは別です。

## 回帰確認

HANDOFFのDay 42回帰セットをすべて実行し、旧部品8・行編集75・分割14・引数74、
assembler 57/44/52、echo各54、loop26ケースとDay 40配置照合を維持しました。
VM自己テスト・数値版/ラベル版cmp、別ビルドVMのmonitor/hello/保存読み込み、xxdも確認しています。
旧サンプルと旧テスト、small-asm、writerは変更していません。

[仕様カード](../specs/043-line-storage.md)を参照してください。
コマンド全文・成功/失敗の生ログと開始時状態は、wp/codex-completion-report.mdとwp/codex-day43-evidence.jsonに保存します。

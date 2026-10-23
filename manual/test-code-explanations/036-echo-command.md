# Day 36: echoの本文を読んで表示する

対象はhost側の `tools/write-echo-command-bin.c` と、生成する `programs/echo-command.bin` です。Day 35のサンプルを残し、既存命令だけでコマンドを増やします。

## 比較対象と処理先を増やす

help、quitに続けてechoと比較します。呼ぶ前にはR1を入力バッファ0x500、R2を比較語echoの0x440に戻します。string_equalがR0へ返す1とR3の1をCMPし、JZでecho処理へ進みます。比較が変更するレジスタを前回のまま使わない点が重要です。

## 同じread_lineを2か所から呼ぶ

以下は説明用表記です。現在のsmall-asmでそのまま組み立てるコードではありません。

```asm
do_echo:
    MOVI R0, 0x460    ; Text> の先頭
    SYSCALL 1
    CALLI read_line
    MOVI R0, 0x500    ; 本文の先頭
    SYSCALL 1
    MOVI R0, 10
    SYSCALL 3
    JUMP 0x000
```

read_lineはコマンド入力時と本文入力時で同じ処理です。LFまで読み、最大63 byteと終端0を置き、CRと超過分を捨てます。echoを選び終えた後なので、元のコマンドを本文で上書きできます。本文を保持したまま別の入力を読む用途では、別のバッファが必要です。

本文側のRETの戻り先は `MOVI R0, 0x500`。ここでは文字列比較を行わず、SYSCALL 1へ渡すためquitも文字として表示します。SYSCALL 1は終端0まで表示し、自動で改行しません。入力のLFも保存していないので、SYSCALL 3でLFを1 byte出力します。

## 命令とデータの配置

| アドレス | 内容 |
|---|---|
| 0x000 | Command>表示、コマンド入力 |
| 0x03C | echoとの比較の準備 |
| 0x058 | help処理 |
| 0x064 | quit処理 |
| 0x070 | unknown処理 |
| 0x07C | echo処理 |
| 0x084 | 本文用CALLI read_line |
| 0x088 | 本文を表示するためR0を設定 |
| 0x098 | コマンド先頭へのJUMP |
| 0x09C | read_line |
| 0x0E0 | string_equal |
| 0x300 / 0x340 | Command> / help表示用文字列 |
| 0x3A0 / 0x3E0 | goodbye / unknown表示用文字列 |
| 0x420 / 0x430 / 0x440 | help / quit / echoの比較語 |
| 0x460 | Text>表示用文字列 |
| 0x500..0x53F | 共用入力バッファ |

70命令、280 byteです。Day 35より比較の6命令とecho処理の8命令が増えます。read_lineとstring_equalの処理は同じでも、前に置く命令が増えたため開始アドレスが変わります。writerはaddress_of(count)で配置を記録し、最後にCALLIや分岐先をpatch_jumpで埋めます。

helpはechoの説明を含め65 byte、終端0を含め66 byte。0x340から0x381に収まり、次のgoodbyeの0x3A0と重なりません。ファイル末尾はText>と終端0の0x466までで、全体は1127 byteです。入力バッファは読み込み後のVMメモリ上で使います。

初期SPは0x100000です。各CALLIで0x0FFFFCになり、RETで元へ戻ります。本文入力のCALLIもコマンド入力・比較のRETが済んでから実行するため、入れ子にはなりません。

## 確認方法

```sh
make run-echo-command
make test-echo-command
```

Python 3の `tools/test-echo-command.py` が実際のVMを起動し、標準出力全体・標準エラー・終了コードを照合します。各起動には5秒の上限があります。

Day 35の入力境界の確認を引き継ぎ、echoの本文、空白、空行、コマンド風の本文、完全一致、短い次行、CRLF、行中CR、NUL、UTF-8、62/63/64/4096 byte、読み捨て、EOF、1,000回の反復、monitor復帰を含む54ケースです。UTF-8のケースは短い本文のbyte列を照合するもので、byte上限で文字を分断しない保証ではありません。

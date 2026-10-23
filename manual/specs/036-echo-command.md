# Day 36: 次の1行を表示するechoコマンド

```text
名前: echo-command.bin
分類: sample program（既存VM命令のみ）
目的: help / quit / unknownのループにechoを追加する
コマンド: help / quit / echoの完全一致。大小文字や空白を変換しない
echo: Text> を表示し、次の1行を読み、保存した文字列とLFを表示してCommand>へ戻る
入力バッファ: 0x500..0x53F（最大63 byteと終端0）、コマンドと本文で共用
read_line: R0〜R2とzero flagを変更し、バッファに結果を残す
string_equal: R1/R2を受け取り、一致ならR0=1、不一致ならR0=0
string_equalの変更: R0〜R4とzero flag
出力: 文字列はSYSCALL 1、本文の後のLFはSYSCALL 3
書くメモリ: 入力バッファとCALLIの戻り先スタック
PC: echo / help / unknownの後は先頭へJUMP、quitでHALT
SP: CALLIで4減り、RETで4戻る。サブルーチンの入れ子なし
EOF: 既存SYSCALL 2でinput EOFを表示して停止。未完の本文は表示しない
```

## 行入力とコマンドの区別

Day 35のread_lineを再利用する。LFで確定し、CRは行中も含め無視する。CR単独では確定しない。最大63 byteを保存し、超過分もLFまで読み捨てる。終端0は毎回その行の末尾へ書く。コマンド行と本文行で同じ規則を使う。

`echo hello` はunknown。`echo`に一致した後で読む行は比較せず表示するので、`quit`や`help`も本文になる。空のコマンド行はunknown、空の本文行はLFだけを表示する。空白は保存する。上限は文字数ではなくbyte数で、UTF-8の途中で切れる場合もある。

NULは拒否せず保存する。コマンド比較と本文表示は最初のNULまでだが、入力自体はLFまで消費する。任意のbyte列をそのまま出力する用途ではない。

## 最初の成功条件

入力 `echo\nhello\nquit\n` に対するpipe実行の出力:

```text
Command> Text> hello
Command> Goodbye from Handmade VM
CPU halted.
```

helpに `echo: read and print a line` を追加する。help / unknown / echoの反復、本文中のquit、空行、前回より短い本文、LF / CRLF、上限の前後、超過分の読み捨て、本文途中のEOF、monitorへの復帰を実際のVMで照合する。

新しい命令・syscall・VM機能は追加しないため、notesの部品テストやVM本体への統合は不要。専用writerと生成した.binをテスト対象にする。

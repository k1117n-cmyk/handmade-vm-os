# Day 35: 複数文字コマンドのループ

```text
名前: word-command-loop.bin
分類: sample program（既存VM命令のみ）
目的: help / unknownの後は再入力し、quitで終了する
入力: host標準入力のLFで終わる行
出力: Command> とhelp / unknown / goodbyeのメッセージ
入力バッファ: 0x500..0x53F（最大63 byteと終端0）
read_line: R0〜R2とzero flagを変更し、入力をバッファに残す
string_equal: R1/R2を受け取り、一致ならR0=1、不一致ならR0=0
string_equalの変更: R0〜R4とzero flag
書くメモリ: 入力バッファ、CALLIの戻り先スタック
PC: help / unknownの表示後はJUMPで先頭へ。quitだけHALT
SP: 各CALLIで4減り、RETで元へ戻る
EOF: 既存SYSCALL 2がinput EOFを表示して停止。未完の行は判定しない
```

## 行入力の約束

LF（10）で入力を確定する。CR（13）は保存せず読み捨てるので、LFとCRLFのどちらも1行になる。CR単独は行末として扱わない。行中のCRも無視する。

63 byteを保存したら、書き込みポインタを0x53Fで止める。その後もLFまで読み続け、余分な文字を捨てる。LFで現在のポインタ位置に0を置いて戻る。63 byteちょうどでもLFを消費する。長すぎる行は先頭63 byteを比較する。空行はunknown。NUL byteは通常の入力として保存するため、比較は最初のNULまでとなる（テキスト入力を想定）。

各呼び出しでR1を0x500へ戻し、最後に必ず終端0を書く。前の行より短い入力でも古い末尾を比較しない。

## 手作りの成功条件

`help\nhello\nquit\n` の出力:

```text
Command> Commands:
help: show help
quit: quit
Command> Unknown command
Command> Goodbye from Handmade VM
CPU halted.
```

CRLFでも同じ出力。62/63/64 byteの行の後にquitを渡すと、unknownは1回だけで終了する。長い行の残りにhelpやquitがあっても次のコマンドにはならない。空行、短い単語、大文字、繰り返し、EOF、monitorへ戻る動作も確認する。

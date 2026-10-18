## Day 34: 文字列比較サブルーチン

```text
名前: word-command-compare.bin
分類: sample program
目的: help / quit の比較に同じVMサブルーチンを使う
命令長: 4 byte固定（既存命令のみ）
入力: R1 = 入力文字列、R2 = 比較する0終端文字列の先頭
出力: R0 = 完全一致なら1、不一致なら0
変更するレジスタ: R0, R1, R2, R3, R4
読むメモリ: R1とR2から始まる0終端文字列
書くメモリ: CALLIによる戻り先のスタック保存のみ
PC: CALLIでstring_equalへ進み、RETで呼び出しの次へ戻る
SP: CALLIで4減り、RETで4戻る
条件フラグ: CMPで変わる。呼び出し側はR0を改めてCMPする
成功条件: Day 33と同じhelp / quit / unknownの出力
```

比較は1 byteずつ行う。異なれば0を返す。同じで、そのbyteが0なら1を返す。それ以外は両方のアドレスを1増やして繰り返す。Cのstrcmpとは異なり、一致時に1を返す。

比較対象はwriterが配置する `0x420` の `help\0` と `0x430` の `quit\0`。入力は従来どおり `0x500` から最大63 byteと終端0。比較でR1/R2が進むので、quit比較前に両方を設定し直す。R3は比較先のbyte、R4は0との比較に使う。

read_lineのLF/CR・63 byte上限・EOF時停止の動作はDay 33を引き継ぐ。上限後の残り入力やCRLFのLFは読み捨てない。任意の未終端文字列を渡す汎用ライブラリにはしない。

手作りの期待結果: `help` はhelp表示、`quit` は終了表示、空行・`hel`・`helpful`・`quits`・63 byteの入力はunknown。入力途中のEOFは `input EOF`。新しいVM命令やsyscallは追加しない。

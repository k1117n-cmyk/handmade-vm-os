# Day 34: 文字列比較を1つのルーチンにする

対象は `tools/write-word-command-compare-bin.c` と、生成する `programs/word-command-compare.bin`。前者はhost側のwriter、後者がVM上で動くサンプルです。新しい命令の部品テストではなく、既存命令を組み合わせる練習です。

## 呼び出し側から追う

Day 33では `h`、`e`、`l`、`p`、0をそれぞれ即値と比較しました。Day 34では `help\0` と `quit\0` をメモリへ置き、同じループで読みます。

```text
0x008: CALLI 0x064       read_lineへ。戻り先は0x00C
0x00C: MOVI R1, 0x500   入力バッファ
0x010: MOVI R2, 0x420   helpの先頭
0x014: CALLI 0x0A8      string_equalへ。戻り先は0x018
0x018: MOVI R3, 1
0x01C: CMP R0, R3
0x020: JZ 0x040         一致したらhelp表示
0x024: MOVI R1, 0x500   比較で進んだポインタを戻す
0x028: MOVI R2, 0x430   quitの先頭
0x02C: CALLI 0x0A8      同じルーチンへ。戻り先は0x030
```

`CALLI 0x0A8` はtype=6、op=9、imm=0x0A8なので `0x690000A8`。big-endianの4 byteは `69 00 00 A8` です。呼び出し先が同じでも、スタックへ保存する戻り先は異なります。各RETでSPは元へ戻ります。

## 比較のループ

次は説明用の表記です。現在のsmall-asmでそのまま組み立てるものではありません。

```asm
string_equal:
    MOVI R4, 0
compare_loop:
    LDB R0, [R1]
    LDB R3, [R2]
    CMP R0, R3
    JNZ not_equal
    CMP R0, R4
    JZ equal
    INC R1
    INC R2
    JUMP compare_loop
equal:
    MOVI R0, 1
    RET
not_equal:
    MOVI R0, 0
    RET
```

入力が `help` なら、R1/R2は `0x500/0x420` から始まり、4回進むと `0x504/0x424` になります。そこは両方0なので、一致として1を返します。入力が `helpful` なら、その位置は `'f'/0` なので0を返します。入力が `hel` なら、その1つ手前の `0/'p'` で不一致です。

先に2 byteの一致を調べてから0終端を調べる順序が大切です。入力側が0という理由だけで一致にすると、短い入力まで受け入れてしまいます。

戻り値はR0に置きます。zero flagの値を呼び出しの約束には含めず、呼び出し側がR0と1をCMPしてからJZします。R0〜R4は作業用で、呼び出し前の値を保存しません。文字列のメモリは変更しません。

## 配置と確認

命令列は `0x000..0x0DF` の224 byteです。read_lineは `0x064`、string_equalは `0x0A8`、比較ループは `0x0AC`、一致時の処理は `0x0D0`、不一致時は `0x0D8`。表示処理はそれぞれHALTで終わるので、そのままサブルーチンへ流れ込みません。

```sh
make test-word-command-compare
printf 'helpful\n' | ./handmade-vm programs/word-command-compare.bin
printf '\n' | ./handmade-vm programs/word-command-compare.bin
xxd programs/word-command-compare.bin
```

Makefileのターゲットはhelp / quit / helloを順に実行して表示します。出力の一致を自動判定するターゲットではありません。helpfulと空行はどちらも `Command> Unknown command`、続いて `CPU halted.` となります。

今回の確認では、help / quit / hello、空行、hel / helpful / quits / HELP、CRとCRLF、62・63・64 byte、空のEOFと途中EOFの計15ケースで、Day 33との標準出力・標準エラー・終了コードの一致を確認しました。monitorから起動し、終了後monitorへ戻ることも確認しています。

入力上限とEOFは従来どおりです。63 byteで戻ると残り入力はhost側に残ります。繰り返しコマンドを受け付ける次の段階では、残り入力やCRLFをどう扱うか決める必要があります。

# Day 35: 入力と比較を繰り返す

対象はhost側の `tools/write-word-command-loop-bin.c` と、生成するVMプログラム `programs/word-command-loop.bin` です。Day 34を残したまま別のサンプルを追加します。VM本体と命令の仕様は変わりません。

## 表示後に先頭へ戻る

Day 34のhelpとunknownは表示後にHALTしていました。今回はその2か所を `JUMP 0x000` に変えます。type=6、op=8、即値=0なので、命令は `0x68000000`、メモリ上のbyte列は `68 00 00 00` です。

```text
0x000: MOVI R0, 0x300   Command> の先頭
0x004: SYSCALL 1
0x008: CALLI 0x064      read_line
       help / quitとstring_equalで比較
0x040: MOVI R0, 0x340   help表示
0x044: SYSCALL 1
0x048: JUMP 0x000       次の入力へ
0x04C: MOVI R0, 0x3A0   goodbye表示
0x050: SYSCALL 1
0x054: HALT
0x058: MOVI R0, 0x3E0   unknown表示
0x05C: SYSCALL 1
0x060: JUMP 0x000       次の入力へ
```

先頭へ戻るのはCALLIではなくJUMPです。入力と比較のCALLIはそれぞれRETで戻ってから表示へ進むため、反復ごとにスタックへ戻り先が積み残されることはありません。

## 保存を止めても読み取りは続ける

Day 34では63 byteを保存すると即座に戻りました。これをそのままループにすると、残りの文字や改行を次のコマンドとして読んでしまいます。今回は「行が終わったか」と「まだ保存できるか」を分けます。

次は説明用の表記であり、現在のsmall-asmで組み立てるものではありません。

```asm
read_line:
    MOVI R1, 0x500
read_loop:
    SYSCALL 2
    MOVI R2, 10
    CMP R0, R2
    JZ finish_input
    MOVI R2, 13
    CMP R0, R2
    JZ read_loop
    MOVI R2, 0x53F
    CMP R1, R2
    JZ read_loop
    STB [R1], R0
    INC R1
    JUMP read_loop
finish_input:
    MOVI R0, 0
    STB [R1], R0
    RET
```

`0x53F - 0x500 = 63` なので、文字を保存するのは `0x500..0x53E` の63 byteまでです。R1が `0x53F` ならSTBとINCを飛ばします。R1を進めず入力だけ読み続け、LFで `0x53F` に終端0を置きます。短い行では、その行の末尾に0を書きます。バッファ全体を消さなくても、前回の文字が比較に混ざりません。

CRは読み捨て、LFで確定します。CRLFで余計な空行が発生せず、quitの後のLFもVM側で消費してからmonitorへ戻れます。この方式ではCR単独は行末にならず、行中のCRも消えます。先読みや入力の持ち越しを追加せず、LF / CRLFのテキスト入力を扱うための約束です。

上限超過の行は先頭63 byteだけを比較します。途中EOFでは、既存のSYSCALL 2が `input EOF` を表示して停止するので、その行は実行しません。空行はunknownです。NULを含む入力は最初のNULまでで比較されます。

## 配置と確認

命令数はDay 34と同じ56命令、224 byteです。入力処理の命令を並べ替えたため、read_lineは `0x064`、string_equalは `0x0A8` のままです。文字列データと入力バッファの位置も同じです。

```sh
make run-word-command-loop
# Command> に help、hello、quit を順に入力する
make test-word-command-loop
```

自動テストにはPython 3を使います。`tools/test-word-command-loop.py` は実際のVMを起動し、標準出力全体・標準エラー・終了コードを期待値と比較します。停止しない不具合も検出するため、各起動の待ち時間を5秒に制限します。

26ケースで、通常の反復、完全一致、空行、CRLF、CR単独、行中CR、前回より短い行、62/63/64/4096 byte、上限後のコマンド風文字列、EOF、1,000回のhelp、monitorへ戻った後のquitを確認します。

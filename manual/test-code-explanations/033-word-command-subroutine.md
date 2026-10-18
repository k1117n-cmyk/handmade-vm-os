## Day 33: writerと入力ルーチンの読み方

`tools/write-word-command-subroutine-bin.c` はVMプログラムを生成するhost側ツール。`notes/*-test.c` やVM本体の変更ではない。

Day 32の `word-command.bin` では、プロンプトのすぐ後に入力ループを並べていた。今回は、その場所に `CALLI read_line` を1命令置く。`read_line` の命令列は、各結果表示の `HALT` の後ろに配置する。通常の実行がそのままルーチンへ流れ込むことはない。

writerは最初に `CALLI` の行き先を空欄にし、ルーチンの先頭アドレスが決まってから `patch_jump` で埋める。`JZ` の終了先も同じ方法で埋める。生成された先頭12 byteは次のとおり。

```text
40 00 03 00   MOVI R0, 0x300
60 00 00 01   SYSCALL 1
69 00 01 00   CALLI 0x100
```

入力ルーチンは `SYSCALL 2` で1 byte読み、LF/CRなら終端へ進む。それ以外は `STB [R1], R0` で保存し、`R1` を増やす。63 byte保存したら終端へ進む。終端では0を書いて `RET` する。`RET` 後は `CALLI` の次の命令から `help` の比較を始める。

この段階では文字列比較はDay 32と同じく各文字を順に比較する。次回の候補は、その比較側を整理すること。

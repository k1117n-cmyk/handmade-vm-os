# Day 40: echoの.asm移行を確かめる

## 動かす

```sh
make run-asm-echo-command
```

echo、Enter、hello、Enter、quit、Enterと入力すると、Text>で受け取ったhelloを表示し、Command>へ戻ってから終了する。自動照合は次のコマンドで行う。

```sh
make test-asm-echo-command
```

既存writer版とasm版をそれぞれ54ケースで実行する。`tools/test-echo-command.py`の第2引数にプログラムを指定できるようにし、期待値は共用する。引数を省略した従来の呼び出しも使える。

## 配置表

アドレスはファイル先頭から数えたbyte位置であり、VMへのロード後も同じ位置になる。

| 領域・ラベル | writer | asm | byte数（終端0込み） |
|---|---:|---:|---:|
| 命令70個 | 0x000 | 0x000 | 280 |
| prompt_text | 0x300 | 0x118 | 10 |
| help_text | 0x340 | 0x122 | 66 |
| goodbye_text | 0x3A0 | 0x164 | 26 |
| unknown_text | 0x3E0 | 0x17E | 17 |
| help_word | 0x420 | 0x18F | 5 |
| quit_word | 0x430 | 0x194 | 5 |
| echo_word | 0x440 | 0x199 | 5 |
| text_prompt | 0x460 | 0x19E | 7 |
| 入力バッファ（ファイル外） | 0x500 | 0x500 | 64 |

asm版は280 + 141 = 421 byte（末尾の次は0x1A5）。writer版は1127 byte（末尾の次は0x467）。どちらも0x500未満に収まる。バッファはVMのメモリにあり、ファイル内に空間を作らなくてもSTBで書き込める。

命令はすべて4 byte境界にあり、最後のRETより後にだけ文字列を置く。文字列は1 byte単位で読まれるため整列は不要。RETが実行されれば呼び出し元へ戻り、文字列へ順次実行が流れ込まない。

## 完全一致するもの、再配置するもの

`tools/test-asm-echo-command.py`は70個の命令をbig endianで読み取る。8個のMOVIについて、writerの旧アドレスとasmの新アドレス、宛先レジスタ、文字列の全byteを照合する。それらの参照値を旧値へ戻したうえで70命令を比較し、他の命令が一致することを確認する。これによりCALLIやJUMPの宛先、0x500と0x53Fの即値も一致する。

全ファイルをcmpすると、文字列アドレスと間の空き領域が違うので不一致になる。今回必要なのは、移動後も同じ文字列を読み、同じ制御の流れで実行すること。

さらに生成物のサイズを0x500以下に制限し、文字列が入力バッファへ入り込む変更を検出する。この検査はサンプル専用で、small-asmが固定バッファを自動保護するわけではない。

## 入出力の検証範囲

54ケースはhelp / quit / unknown、echo本文、空行、空白保持、CRLF、CR単独、62/63/64/4096 byte、EOF、埋め込みNUL、UTF-8、1000回反復、monitorからの実行を含む。ASCII制限は.asm中の.stringに対するものであり、実行時の入力はbyte列として扱う。

## 結果

70命令・8文字列・421 byteの配置照合と、writer版54ケース、asm版54ケースが通過。VM本体とsmall-asmの変更は不要だった。仕様は[040-asm-echo-command.md](../specs/040-asm-echo-command.md)。

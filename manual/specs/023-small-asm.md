## Day 23: Small Assembler

```text
名前: small assembler
分類: 開発補助ツール
目的: 最小限のアセンブリ表記を、VMが読み込めるbig-endianの外部バイナリへ変換する
命令長: なし。命令ではなくホスト側の作成ツール
bit配置: 出力する各命令は既存の32bit命令フォーマットに従う
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: なし
条件フラグの変化: なし
エラー時: 入力ファイルを開けない、出力ファイルを開けない、未対応命令、引数不正、行が長すぎる場合は終了
手作りテスト: tools/small-asm.c で programs/hello.asm から /tmp/hello-small-asm.bin を作る
成功条件: 生成したバイナリをVMで実行すると A と CPU halted. が表示される
```

Day 23 では、本格的なアセンブラではなく、外部バイナリを少しだけ作りやすくする小さな変換ツールを追加する。

最初に扱う命令は次の3つだけにする。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

対応する命令値は次の通り。

```text
MOVI R0, 65  -> 0x40000041
SYSCALL 0    -> 0x60000000
HALT         -> 0x01000000
```

`tools/small-asm.c` は、入力ファイルを1行ずつ読み、対応する32bit命令値をbig-endianで出力する。

```sh
cc tools/small-asm.c -o /tmp/small-asm
/tmp/small-asm programs/hello.asm /tmp/hello-small-asm.bin
```

生成したバイナリは、VMに渡して実行できる。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm /tmp/hello-small-asm.bin
```

期待する出力:

```text
A
CPU halted.
```

## Day 27時点の対応命令

Day 27 の `one-char-command.asm` で1文字コマンド判定を書くため、`tools/small-asm.c` は次の命令にも対応する。

```asm
CMP R0, R1
JUMP 0x10
JZ 0x20
JNZ 0x30
```

この時点では、まだラベルは扱わない。分岐先は即値アドレスで書く。

命令値:

```text
CMP R0, R1 -> 0x24010000
JUMP 0x10 -> 0x68000010
JZ 0x20   -> 0x6A000020
JNZ 0x30  -> 0x6B000030
```

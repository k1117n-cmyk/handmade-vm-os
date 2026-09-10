## Day 22: Hello Binary Writer

```text
名前: hello.bin 作成ツール
分類: 開発補助ツール
目的: 最小プログラムを表す3つの32bit命令を、big-endianの外部バイナリとして書き出す
命令長: なし。命令ではなくホスト側の作成ツール
bit配置: 出力する各命令は既存の32bit命令フォーマットに従う
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: なし
条件フラグの変化: なし
エラー時: 出力ファイルを開けない、書けない場合は終了
手作りテスト: tools/write-hello-bin.c で programs/hello.bin を作る
成功条件: programs/hello.bin をVMで実行すると A と CPU halted. が表示される
```

Day 21 では、VMが外部バイナリを読み込めるようになりました。

Day 22 では、その外部バイナリを作る方法を小さく用意します。

まだ本格的なアセンブラは作りません。まずは、次の3命令だけをファイルへ書き出します。

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

命令値は次の通りです。

```text
MOVI R0, 65  -> 0x40000041
SYSCALL 0    -> 0x60000000
HALT         -> 0x01000000
```

このVMの命令は big-endian でメモリへ置くので、ファイルにも同じ順番で書きます。

```text
0x40000041 -> 40 00 00 41
0x60000000 -> 60 00 00 00
0x01000000 -> 01 00 00 00
```

`tools/write-hello-bin.c` は、この3つの値を `programs/hello.bin` へ書き出します。

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
```

生成したバイナリは、VMに渡して実行できます。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待する出力:

```text
A
CPU halted.
```

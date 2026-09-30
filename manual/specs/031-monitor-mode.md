## Day 31: monitor mode

```text
名前: monitor mode
分類: VM起動機能
目的: 引数なしでVMを起動したとき、host側のmonitorから実行する外部バイナリを選べるようにする
命令長: なし。命令ではなくVM本体の起動時処理
bit配置: なし
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: 選択した program.bin を実行するたびに、新しいVMの memory[0] から読み込む
PCの変化: 外部バイナリ実行ごとに PC = 0x00000000 から開始する
条件フラグの変化: 外部バイナリ実行ごとに初期化する
エラー時: ファイルを開けない場合はmonitorへ戻る
成功条件: ./handmade-vm で Welcome と > を表示し、入力した .bin を実行し、HALT後にmonitorへ戻る
```

## 役割

`monitor mode` は、まだVM上のOSではない。

host側の `notes/vm.c` が、標準入力からファイル名を読み、指定された外部バイナリを `memory[0]` から読み込んで実行する。

```text
host側monitor:
  ファイル名を読む
  program.bin を開く
  VMのmemoryへ読み込む
  VMを実行する

VM上のプログラム:
  memory[0] からfetchされる命令列として動く
```

この段階では、VM上のOSがhostファイルを読んでいるわけではない。

## 起動方法

引数なしの場合はmonitorを起動する。

```sh
./handmade-vm
```

表示:

```text
Welcome to Handmade VM
>
```

プロンプトで外部バイナリのパスを入力する。

```text
>programs/hello.bin
```

`quit`, `q`, `exit` のいずれかを入力するとmonitorを終了する。

```text
>quit
Goodbye from Handmade VM
```

## 従来の起動方法

引数に外部バイナリを渡す実行方法は残す。

```sh
./handmade-vm programs/hello.bin
```

内蔵テストプログラムは `--self-test` に移す。

```sh
./handmade-vm --self-test
```

## 自動確認

```sh
make test-monitor
```

期待出力:

```text
Welcome to Handmade VM
>A
CPU halted.
>Type a word, then Enter
>You typed: help
CPU halted.
>Goodbye from Handmade VM
```

パイプで入力した場合、端末側の入力echoがないため、monitorの `>` と実行したプログラムの出力が同じ行に続いて見える。

## monitor mode の解説

`monitor mode` は、引数なしで `./handmade-vm` を起動したときに動くhost側の入口です。

これまでは、引数なしで起動すると `load_test_program()` が内蔵テスト命令列を `memory` へ置いていました。

```sh
./handmade-vm
```

```text
A
VM flow complete.
CPU halted.
```

Day 31 からは、引数なし起動ではmonitorを表示します。

```text
Welcome to Handmade VM
>
```

## まだOSではない

ここで大事なのは、monitorがVM上のOSではないことです。

monitorはhost側のCコードです。ユーザーが入力したパスをhost側で読み、そのファイルを開き、新しいVMの `memory[0]` へ読み込んでから実行します。

```text
./handmade-vm
  -> host側monitor
  -> programs/hello.bin を開く
  -> 新しいVMへ読み込む
  -> VMを実行する
```

VM上のプログラムからhostファイルを開いているわけではありません。

## 1回ごとにVMを初期化する

monitorから外部バイナリを実行するときは、毎回新しい `VM` を初期化します。

```c
init_vm(&vm);
load_program_file(&vm, path);
run(&vm);
```

これにより、前に実行したプログラムの `memory` や `regs` が次のプログラムへ残りません。

## 終了コマンド

monitorでは、次の入力を終了コマンドとして扱います。

```text
quit
q
exit
```

終了時には次の文字列を表示します。

```text
Goodbye from Handmade VM
```

## 内蔵テスト

これまで引数なしで動いていた内蔵テストは、`--self-test` へ移します。

```sh
./handmade-vm --self-test
```

これにより、学習用の統合VM確認は残したまま、引数なし起動をmonitorの入口として使えるようになります。

## Day 21: Binary Loader

```text
名前: 外部バイナリローダー
分類: VM起動機能
目的: VM外部の program.bin を memory[0] から読み込み、PC=0 で実行する
命令長: なし。命令ではなくVMの起動時処理
bit配置: なし
読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: memory[0] から、読み込んだbyte数ぶん
PCの変化: 起動時 PC = 0x00000000。fetch後は通常通り +4
条件フラグの変化: なし
エラー時: ファイルを開けない、読めない、1MBを超える場合は実行せず終了
手作りテスト: programs/hello.bin に MOVI R0, 65 / SYSCALL 0 / HALT を置く
成功条件: ./vm programs/hello.bin で A と CPU halted. が表示される
```

これまでの `notes/vm.c` は、Cコードの中で `load_test_program()` を呼び、命令を直接メモリへ置いていました。

```c
write_inst(vm, 0x00000000, 0x40000041);
write_inst(vm, 0x00000004, 0x60000000);
write_inst(vm, 0x00000008, 0x01000000);
```

Day 21 では、この命令列をVMの外に出します。

```text
programs/hello.bin
  40 00 00 41
  60 00 00 00
  01 00 00 00
```

VMは `programs/hello.bin` の中身を `memory[0]` から順番に読み込みます。

```text
memory[0x00000000] = 0x40
memory[0x00000001] = 0x00
memory[0x00000002] = 0x00
memory[0x00000003] = 0x41
...
```

その後の `fetch`, `decode`, `execute` はこれまでと同じです。

```text
PC = 0
fetch 4 byte
decode
execute
```

この段階では、本格的なアセンブラはまだ作りません。

まずは、手で作れる小さなバイナリをVMが読めるようにすることを目的にします。

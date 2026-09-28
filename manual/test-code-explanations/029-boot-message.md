## boot-message.bin の解説

`boot-message.bin` は、VM上の外部プログラムに、命令だけでなく文字列データも入れる練習です。

起動時に welcome メッセージを表示し、そのあと1文字コマンドループへ入ります。

```text
h: helpを表示してプロンプトへ戻る
q: 終了メッセージを表示してHALTする
Enter / space: 何も表示せず次の入力を待つ
その他: ? を表示してプロンプトへ戻る
```

今回の目的は、次の2つを分けて見ることです。

```text
PCが読む場所: 命令
R0が指す場所: 文字列データ
```

## メモリ配置

バイナリの先頭には、次の命令列を置きます。

```text
0x00000000: 40 00 01 00    MOVI R0, 0x100
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 40 00 00 3E    MOVI R0, 62
0x0000000C: 60 00 00 03    SYSCALL 3
0x00000010: 60 00 00 02    SYSCALL 2
...
0x00000064: 40 00 01 40    MOVI R0, 0x140
0x00000068: 60 00 00 01    SYSCALL 1
0x0000006C: 68 00 00 08    JUMP 0x08
0x00000070: 40 00 01 80    MOVI R0, 0x180
0x00000074: 60 00 00 01    SYSCALL 1
0x00000078: 01 00 00 00    HALT
```

`0x100` 以降には起動メッセージを置きます。

```text
0x00000100: Welcome to Handmade VM\n\0
```

`0x140` 以降にはhelp用の文字列を置きます。

```text
0x00000140: Commands:\nh: help\nq: quit\n\0
```

`0x180` 以降には終了メッセージを置きます。

```text
0x00000180: Goodbye from Handmade VM\n\0
```

命令列と文字列の間には使わない領域があります。この空きは、命令が少し増えても文字列の位置を変えずに済むようにするためです。

## MOVI R0, 0x100

最初の命令は、起動メッセージの先頭アドレスを `R0` に入れます。

```asm
MOVI R0, 0x100
```

命令値は次の通りです。

```text
0x40000100
```

decodeすると、次の意味になります。

```text
type = 4
op   = 0
rd   = 0
imm  = 0x100
```

C実装では、次の処理に対応します。

```c
vm->regs[inst.rd] = inst.imm;
```

したがって、この時点で `R0 = 0x100` になります。

## SYSCALL 1

次の命令は、`R0` が指す0終端文字列を表示します。

```asm
SYSCALL 1
```

命令値は次の通りです。

```text
0x60000001
```

`inst.imm == 1` なので、VMは文字列表示の処理へ進みます。

```c
} else if (inst.imm == 1) {
    print_string(vm, vm->regs[0]);
}
```

`print_string` は、指定されたアドレスから1 byteずつ読み、0 byte に到達するまで表示します。

```c
static void print_string(VM *vm, uint32_t address) {
    while (address < MEMORY_SIZE && vm->memory[address] != 0) {
        putchar(vm->memory[address]);
        address++;
    }
}
```

このとき `R0 = 0x100` なので、`memory[0x100]` から `Welcome to Handmade VM\n` が表示されます。

## hでhelp文字列を表示する

起動メッセージを表示したあと、プログラムはプロンプトを表示して1文字読みます。

```asm
MOVI R0, 62
SYSCALL 3
SYSCALL 2
```

読み込んだ文字が `h` なら、`R0` に `0x140` を入れてhelp文字列を表示します。

```asm
MOVI R0, 0x140
SYSCALL 1
```

これにより、`memory[0x140]` から help 用の文字列が表示されます。

```text
Commands:
h: help
q: quit
```

表示後は `JUMP 0x08` でプロンプト表示へ戻ります。

## qで終了メッセージを表示する

読み込んだ文字が `q` なら、`R0` に `0x180` を入れて終了メッセージを表示します。

```asm
MOVI R0, 0x180
SYSCALL 1
HALT
```

これにより、`memory[0x180]` から次の文字列が表示されます。

```text
Goodbye from Handmade VM
```

そのあと `HALT` に到達し、VMが停止します。

## 生成ツールを見る

`tools/write-boot-message-bin.c` では、命令をbig-endianで書き込みます。

```c
write_u32_be(file, 0x40000100);  // MOVI R0, 0x100
write_u32_be(file, 0x60000001);  // SYSCALL 1
```

そのあと、`fseek` で文字列を置きたい場所へ移動します。

```c
write_string_at(file, 0x100, "Welcome to Handmade VM\n");
write_string_at(file, 0x140, "Commands:\nh: help\nq: quit\n");
write_string_at(file, 0x180, "Goodbye from Handmade VM\n");
```

`write_string_at` は文字列本体を書いたあと、最後に `0x00` を1 byte追加します。

この `0x00` があるため、`SYSCALL 1` は文字列の終わりを判断できます。

## 実行結果

```sh
make run-boot-message
```

期待出力:

```text
Welcome to Handmade VM
>h
Commands:
h: help
q: quit
>>q
Goodbye from Handmade VM
CPU halted.
```

ここまでで、VM上の外部プログラムは、単発の1文字表示だけでなく、まとまった文字列をデータとして持ち、1文字コマンドから呼び出せるようになります。

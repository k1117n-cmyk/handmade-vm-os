## SYSCALL print_char_raw

```text
名前: SYSCALL 3
分類: syscall
目的: R0の下位8bitを1文字として、改行なしでhost標準出力へ表示する
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number = 3
読むレジスタ: R0
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: なし
成功条件: R0の下位8bitが、改行なしで表示される
```

`SYSCALL 0` は、R0の下位8bitを1文字として表示したあと、改行も表示する。

プロンプト `>` を表示する場合は、改行が入ると次の入力が別行に移動してしまう。

```text
>
h
```

そこで `SYSCALL 3` は、改行なしで1文字だけ表示する。

```asm
MOVI R0, 62
SYSCALL 3
```

これにより、ターミナル上では次のように入力できる。

```text
>h
```

## C実装

`notes/vm.c` では、`SYSCALL 3` を次のように扱う。

```c
} else if (inst.imm == 3) {
    putchar(vm->regs[0] & 0xFF);
}
```

`SYSCALL 0` との違いは、`putchar('\n')` を呼ばないこと。

## command-loopでの使い方

`programs/command-loop.asm` では、プロンプト表示だけを `SYSCALL 3` にする。

```asm
MOVI R0, 62
SYSCALL 3
SYSCALL 2
```

`H` や `?` の表示には、これまで通り `SYSCALL 0` を使う。これにより、結果表示のあとには改行され、次のプロンプトが見やすくなる。

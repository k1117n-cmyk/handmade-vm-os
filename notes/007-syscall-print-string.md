## Day 7: SYSCALL print_string

名前: SYSCALL print_string
分類: syscall
目的: VM内メモリの0終端文字列をhost端末へ出す
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number = 1
読むレジスタ: R0
書くレジスタ: なし
読むメモリ: memory[R0] から 0 byte まで
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: 未実装syscallなら番号を表示して停止
手作りテスト: SYSCALL 1 -> 0x60000001
成功条件: R0が指す0終端文字列が表示される

テストプログラム:

```text
MOVI R0, 0x10
SYSCALL 1
HALT
```

メモリ配置:

```text
0x00000000: 40 00 00 10    MOVI R0, 0x10
0x00000004: 60 00 00 01    SYSCALL 1
0x00000008: 01 00 00 00    HALT

0x00000010: 48 65 6C 6C 6F 20 66 72
0x00000018: 6F 6D 20 48 61 6E 64 6D
0x00000020: 61 64 65 20 4F 53 0A 00
```

期待する出力:

```text
Hello from Handmade OS
CPU halted.
```

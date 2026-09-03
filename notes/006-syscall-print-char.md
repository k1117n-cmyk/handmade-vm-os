## Day 6: SYSCALL print_char

名前: SYSCALL print_char
分類: syscall
目的: VM内の値をhost端末へ1文字出す
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number = 0
読むレジスタ: R0
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: なし
エラー時: 未実装syscallなら番号を表示して停止
手作りテスト: SYSCALL 0 -> 0x60000000
成功条件: R0の下位8bitが1文字として表示される

テストプログラム:

```text
MOVI R0, 65
SYSCALL 0
HALT
```

期待する出力:

```text
A
CPU halted.
```

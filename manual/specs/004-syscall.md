## SYSCALL

```text
名前: SYSCALL
分類: control immediate instruction
目的: VM外側のサービスを呼ぶ
命令長: 4 byte
bit配置:
  bits 31..28: type = 6
  bits 27..24: op = 0
  bits 19..0 : syscall number
読むレジスタ: syscallごとに決める。最初はR0
書くレジスタ: syscallごとに決める
読むメモリ: syscallごとに決める
書くメモリ: syscallごとに決める
PCの変化: fetch時に +4
条件フラグの変化: 最初は変えない
エラー時: 未実装syscallなら番号を表示して停止
手作りテスト: SYSCALL 1 -> 0x60000001
成功条件: R0が指す0終端文字列を表示する
```

現在のsyscall:

```text
SYSCALL 0: R0の下位8bitを1文字として表示する
SYSCALL 1: R0が指す0終端文字列を表示する
SYSCALL 2: host標準入力から1 byte読み、R0へ入れる
```

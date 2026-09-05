## MOVI

```text
名前: MOVI
分類: immediate instruction
目的: レジスタに20bit即値を入れる
命令長: 4 byte
bit配置:
  bits 31..28: type = 4
  bits 27..24: op = 0
  bits 23..20: destination register
  bits 19..0 : immediate
読むレジスタ: なし
書くレジスタ: destination register
読むメモリ: fetch時にPCから4 byte
書くメモリ: なし
PCの変化: fetch時に +4
条件フラグの変化: 最初は変えない。必要なら後で設計する
エラー時: register number が範囲外なら停止
手作りテスト: MOVI R7, 0x80010 -> 0x40780010
成功条件: 実行後 R7 = 0x80010
```

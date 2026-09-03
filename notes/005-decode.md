## Day 4: Decode

名前: decode
分類: instruction decode 
目的: 32bit instruction から `type`, `op`, `rd`, `imm` を取り出す
命令長: 4 bytes
bit配置:
    bits 31..28: type
    bits 27..24: op
    bits 23..20: rd
    bits 19..0 : imm

読むレジスタ: なし
書くレジスタ: なし
読むメモリ: なし
書くメモリ: なし
PCの変化: なし
条件フラグの変化: なし
エラー時: なし
手作りテスト: instruction = 0x40780010 を decode する
成功条件: type=4, op=0, rd=7, imm=0x80010 が取り出せる

decode は fetch 済みの32bit値を分解するだけ

  例:

  instruction = 0x40780010

  b0 = 0x40
  b1 = 0x78

  type = 4
  op   = 0
  rd   = 7
  imm  = 0x80010

  取り出し方:

  type = (instruction >> 28) & 0x0F;
  op   = (instruction >> 24) & 0x0F;
  rd   = (instruction >> 20) & 0x0F;
  imm  = instruction & 0x000FFFFF;

  確認:

  cc 005-decode-test.c -o 005-decode-test
  ./005-decode-test

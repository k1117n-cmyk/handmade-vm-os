### fetch

- byte-addressed memory: 1アドレス = 1バイト
- instruction fetch byte order: big-endian
- fetch: PCが指す4 bytesを命令として読む 
- opcode: 命令のbyte0
- after fetch: PC += 4

今の段階では、002-fetch で大事なのはこの3つです。

PCが指す場所から4バイト読む
01 00 00 00 を 0x01000000 として組み立てる
読んだら PC を 4 進める

ここが分かれば、次の decode で type / op / rd / imm に分ける準備はできています。
 


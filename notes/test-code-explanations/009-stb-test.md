## 009-stb-test.c の解説

`009-stb-test.c` は、小さな自作VM上で `STB`、つまり「1バイトをメモリへ保存する命令」が正しく動くかを確認するテストです。

全体としては、次の命令列をメモリに直接書き込んで実行します。

```asm
MOVI R1, 0x20      ; R1 = 0x20
MOVI R0, 65        ; R0 = 65 = 0x41 = 'A'
STB [R1], R0       ; memory[R1] = R0 の下位8bit
LDB R2, [R1]       ; R2 = memory[R1]
HALT
```

つまり、`R1` にアドレス `0x20` を入れ、`R0` に `0x41` を入れ、`STB` で `memory[0x20]` に `0x41` を保存します。その後 `LDB` で同じ場所から読み戻し、`R2` に `0x41` が入るかを確認します。

## VMの状態

冒頭でVMの状態を用意しています。

```c
uint8_t memory[1024 * 1024] = {0};
uint32_t pc = 0x00000000;
uint32_t regs[8] = {0};
bool running = true;
```

意味は次の通りです。

- `memory`: 1MBのメモリ。1要素が `uint8_t` なので1バイト単位
- `pc`: プログラムカウンタ。次に読む命令のアドレス
- `regs`: 32bitレジスタ8本、`R0` から `R7`
- `running`: VMの実行継続フラグ

## 命令の読み取り

VMは1命令を4バイト固定長として読んでいます。

```c
uint32_t inst =
    ((uint32_t)memory[pc] << 24) |
    ((uint32_t)memory[pc + 1] << 16) |
    ((uint32_t)memory[pc + 2] << 8) |
    ((uint32_t)memory[pc + 3]);

pc += 4;
```

メモリ上の4バイトをビッグエンディアンとして `uint32_t` に合成しています。

たとえば、次の4バイトは、

```c
memory[0] = 0x40;
memory[1] = 0x10;
memory[2] = 0x00;
memory[3] = 0x20;
```

命令値としては `0x40100020` になります。

## 命令フォーマット

命令は次のように分解されます。

```c
uint8_t type = (inst >> 28) & 0x0F;
uint8_t op = (inst >> 24) & 0x0F;
uint8_t rd = (inst >> 20) & 0x0F;
uint8_t rs = (inst >> 16) & 0x0F;
uint32_t imm = inst & 0x000FFFFF;
```

つまり32bit命令を次のように見ています。

```text
[ type:4 ][ op:4 ][ rd:4 ][ rs:4 ][ imm/unused:16 or imm:20 ]
```

このテストで使う命令は主に3種類です。

```c
type == 4 && op == 0  // MOVI
type == 3 && op == 1  // STB
type == 3 && op == 0  // LDB
```

## MOVI

```c
} else if (type == 4 && op == 0) {
    regs[rd] = imm;
}
```

`MOVI` は即値 `imm` をレジスタ `rd` に入れます。

最初の命令 `0x40100020` は次のように分解されます。

```text
type = 4
op   = 0
rd   = 1
imm  = 0x20
```

したがって、実行内容は次の通りです。

```c
regs[1] = 0x20;
```

つまり `R1 = 0x20` になります。

次の `0x40000041` は次の実行になります。

```c
regs[0] = 0x41;
```

つまり `R0 = 65` になります。

## STB

このテストで一番重要なのはここです。

```c
} else if (type == 3 && op == 1) {
    ...
    memory[regs[rd]] = regs[rs] & 0xFF;
}
```

`STB [R1], R0` の命令は `0x31100000` です。

分解すると次のようになります。

```text
type = 3
op   = 1
rd   = 1
rs   = 0
```

このVMでは `STB` の `rd` を「アドレスを持つレジスタ」、`rs` を「保存する値を持つレジスタ」として使っています。

したがって、実行内容は次の通りです。

```c
memory[regs[1]] = regs[0] & 0xFF;
```

この時点で、

```c
regs[1] == 0x20
regs[0] == 0x41
```

なので、実際には次の保存が行われます。

```c
memory[0x20] = 0x41;
```

`& 0xFF` しているのは、`STB` が Store Byte、つまり1バイトだけ保存する命令だからです。`regs[0]` は32bitですが、メモリは `uint8_t` なので下位8bitだけ保存します。

## LDB

次に、保存した値を読み戻します。

```c
} else if (type == 3 && op == 0) {
    ...
    regs[rd] = memory[regs[rs]];
}
```

`LDB R2, [R1]` は `0x30210000` です。

分解すると次のようになります。

```text
type = 3
op   = 0
rd   = 2
rs   = 1
```

したがって、実行内容は次の通りです。

```c
regs[2] = memory[regs[1]];
```

つまり次の処理です。

```c
regs[2] = memory[0x20];
```

先ほど `STB` によって `memory[0x20]` は `0x41` になっているので、`R2` にも `0x41` が入ります。

## HALT

```c
if (inst == 0x01000000) {
    running = false;
}
```

命令が `0x01000000` なら実行ループを終了します。

## 最後の判定

実行後に状態を表示し、次の条件を確認します。

```c
if (memory[0x20] == 0x41 && regs[2] == 0x41) {
    printf("STB test passed.\n");
    return 0;
}
```

確認しているのは次の2点です。

- `STB` によって `memory[0x20]` に `0x41` が保存されたか
- `LDB` によって `R2` に `0x41` を読み戻せたか

要するにこのコードは、`R0` の下位1バイトを `R1` が指すメモリアドレスへ保存し、それを読み戻して一致するかを見る、`STB` 命令の最小テストです。

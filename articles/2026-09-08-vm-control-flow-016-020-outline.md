# 自作VMに計算・比較・分岐・CALL/RETを追加する記事構成案

## 想定タイトル

```text
自作VMに ADD / SUB / CMP / JUMP / JZ / JNZ / CALLI / RET を追加して、プログラムの流れを変えられるようにする
```

別案:

```text
自作VMを「上から順番に実行するだけ」から一歩進める: 計算、比較、分岐、関数呼び出し
```

```text
C言語で作る小さなVM: ADD/SUBからCALL/RETまで実装する
```

## この記事で扱う範囲

この記事では、Day 16 から Day 20 までで追加した命令と、統合VM `notes/vm.c` への `SYSCALL 1` 統合をまとめます。

扱う命令:

```text
Day 16: ADD / SUB
Day 17: CMP
Day 18: JUMP / JZ
Day 19: JNZ
Day 20: CALLI / RET
```

最後に扱う統合:

```text
SYSCALL 1:
  R0が指す0終端文字列を表示する

notes/vm.c:
  A
  VM flow complete.
  CPU halted.
```

## 読者に持って帰ってもらうこと

- `ADD` / `SUB` を入れると、VMの中でレジスタ同士の計算ができます
- `CMP` はレジスタを書き換えず、比較結果だけを `zero_flag` に残します
- `JUMP` / `JZ` / `JNZ` は `PC` を書き換えて、実行順序を変えます
- `CALLI` / `RET` は、戻り先PCをスタックに積むことでサブルーチンを作ります
- `SYSCALL 1` を統合VMに入れると、メモリ上の文字列を表示できます

## 導入

前回までで、VMは値を置く、読む、書く、スタックへ退避するところまで進みました。

```text
MOVI
MOV
LDB / STB
LDDI / STDI
PUSH / POP
```

ただし、この段階のVMはまだ、基本的には上から順番に命令を実行するだけです。

今回の目標は、VMに次の4つの力を足すことです。

```text
1. 計算する
2. 比較する
3. 条件によって実行位置を変える
4. サブルーチンを呼んで戻る
```

この4つが入ると、小さなプログラムの構造がかなり見えやすくなります。

## 第1章: Day 16 ADD / SUB

### 章の狙い

レジスタ同士の足し算と引き算を追加します。

### 命令割り当て

```text
ADD: type=2, op=2
SUB: type=2, op=3
```

### 入れるアセンブリ例

```asm
MOVI R0, 10
MOVI R1, 3
ADD R0, R1

MOVI R2, 10
MOVI R3, 3
SUB R2, R3
HALT
```

### 入れるCコード例

```c
regs[rd] += regs[rs];
regs[rd] -= regs[rs];
```

### 説明するポイント

- `ADD R0, R1` は `R0 = R0 + R1`
- `SUB R2, R3` は `R2 = R2 - R3`
- `rd` は読み書きするレジスタ
- `rs` は読むだけのレジスタ
- この時点では条件フラグ更新はまだ扱わない

### 実行結果

```text
R0=0x0000000D
R2=0x00000007
ADD/SUB test passed.
```

## 第2章: Day 17 CMP と zero_flag

### 章の狙い

条件分岐の準備として、2つのレジスタを比較する `CMP` を追加します。

### 命令割り当て

```text
CMP: type=2, op=4
```

### VM状態に追加するもの

```c
bool zero_flag;
```

### 入れるCコード例

```c
vm->zero_flag = vm->regs[inst.rd] == vm->regs[inst.rs];
```

### 説明するポイント

- `CMP` はレジスタの値を変えない
- 同じ値なら `zero_flag = true`
- 違う値なら `zero_flag = false`
- 次の `JZ` / `JNZ` がこの結果を使う

### 実行結果

```text
first_cmp_zero=true
second_cmp_zero=false
CMP test passed.
```

## 第3章: Day 18 JUMP / JZ

### 章の狙い

`PC` を書き換えて、実行順序を変える命令を追加します。

### 命令割り当て

```text
JUMP: type=6, op=8
JZ:   type=6, op=10
```

### 入れるCコード例

```c
vm->pc = inst.imm;
```

```c
if (vm->zero_flag) {
    vm->pc = inst.imm;
}
```

### 説明するポイント

- `fetch` で `PC` は一度 `+4` される
- `JUMP` はその後で `PC` を上書きする
- `JZ` は `zero_flag == true` のときだけ `PC` を上書きする
- ジャンプ先が命令をfetchできる範囲か確認する

### 実行結果

```text
R0=0x00000001
R1=0x00000001
R2=0x00000007
zero_flag=true
JUMP/JZ test passed.
```

## 第4章: Day 19 JNZ

### 章の狙い

`JZ` の逆として、`zero_flag` が立っていないときにジャンプする `JNZ` を追加します。

### 命令割り当て

```text
JNZ: type=6, op=11
```

### 入れるCコード例

```c
if (!vm->zero_flag) {
    vm->pc = inst.imm;
}
```

### 説明するポイント

- `JZ` は同じだったら飛ぶ
- `JNZ` は違っていたら飛ぶ
- `CMP`、`JZ`、`JNZ` で if 文に近い形が作れる

### 実行結果

```text
R0=0x00000001
R1=0x00000002
R2=0x00000007
zero_flag=false
JNZ test passed.
```

## 第5章: Day 20 CALLI / RET

### 章の狙い

スタックを使って、サブルーチンを呼んで戻る命令を追加します。

### 命令割り当て

```text
CALLI: type=6, op=9
RET:   inst == 0x02000000
```

### CALLI の動き

```text
1. fetch後のPCを戻り先として使う
2. SPを4減らす
3. memory[SP..SP+3] に戻り先PCを書く
4. PC = imm
```

### RET の動き

```text
1. memory[SP..SP+3] から戻り先PCを読む
2. SPを4増やす
3. PC = return_address
```

### 入れるCコード例

```c
vm->sp -= 4;
write_u32_be(vm->memory, vm->sp, vm->pc);
vm->pc = inst.imm;
```

```c
uint32_t return_address = read_u32_be(vm->memory, vm->sp);
vm->sp += 4;
vm->pc = return_address;
```

### 説明するポイント

- `CALLI` は `JUMP` と違い、戻り先を覚える
- 戻り先は `CALLI` の次の命令アドレス
- 戻り先PCはスタックへ積む
- `RET` はスタックから戻り先PCを取り出す

### 実行結果

```text
R0=0x0000002A
R1=0x00000007
SP=0x00100000
CALLI/RET test passed.
```

## 第6章: 統合VMに SYSCALL 1 を入れる

### 章の狙い

これまで個別テストでは動いていた `SYSCALL 1` を、統合VM `notes/vm.c` に入れます。

### 入れるCコード例

```c
static void print_string(VM *vm, uint32_t address) {
    while (address < MEMORY_SIZE && vm->memory[address] != 0) {
        putchar(vm->memory[address]);
        address++;
    }
}
```

```c
} else if (inst.imm == 1) {
    print_string(vm, vm->regs[0]);
}
```

### 説明するポイント

- `SYSCALL 0` は `R0` の下位1 byteを文字として表示する
- `SYSCALL 1` は `R0` をアドレスとして使う
- `memory[R0]` から0終端までを文字列として表示する
- `PC` は命令を読む場所、`R0` はデータを読む場所

### 統合VMの実行結果

```text
A
VM flow complete.
CPU halted.
```

## まとめ

今回で、VMはかなり「プログラムを実行している」形に近づきました。

```text
ADD / SUB:
  計算する

CMP:
  比較する

JUMP / JZ / JNZ:
  実行位置を変える

CALLI / RET:
  呼び出して戻る

SYSCALL 1:
  メモリ上の文字列を表示する
```

まだ外部の `.bin` を読み込むVMにはなっていません。

今の `notes/vm.c` は、内蔵テストプログラムを実行する統合VMです。

次に進むなら、次のどちらかが候補になります。

```text
1. 外部バイナリローダーを追加して、./vm program.bin の形にする
2. lesson 04 OS に必要な SBT / SBTI / NOT / 入力系syscall へ進む
```

記事としては、ここで一区切りにする。

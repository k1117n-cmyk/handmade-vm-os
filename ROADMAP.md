# Handmade VM Roadmap

## Goal

自作CPUの上に自作OSを載せ、その上で自作シェルを動かし、最後に自作エディターを開ける仮想マシンを完成させる。

手書きで学習しながら進める具体的な作業方法は `HANDWRITING_GUIDE.md` にまとめる。

最終的な構成は次の形を目指す。

```text
host macOS/Linux
  -> 自作VM本体 C
  -> 自作CPU ISA
  -> 自作OS
  -> 自作shell
  -> 自作editor
```

参考にする既存教材とコード:

- CPU/VM: `../LC-3VM/lc3.c`
- OS: `../os_dev/os_book_code/*/os.asm`
- Shell: `Tutorial - Write a Shell in C`
- Editor: `kilo`

これらは、今回の企画で組み合わせて使う部品ではない。

今回の企画では、自作CPU、自作VM、自作OS、自作シェル、自作エディターを、学習しながら一から手書きで組み上げる。既存教材は、仕様や実装で迷ったときに後から照合するための参考資料として扱う。

つまり、最初から `lc3.c` や `emu.py` を土台にして移植するのではなく、現在の `handmade-vm-roadmap` の中で、1命令、1syscall、1機能ずつ小さく設計して実装する。

## Direction

この企画の中心は、既存コードの移植ではなく、手書きで理解できるVMを育てること。

学習用VMの現在仕様は `manual/specs/000-machine-state.md` を基準にする。

- 1MB memory
- 32bit instruction
- 20bit address space
- `R0`-`R7`, `SP`, `PC`
- `SYSCALL`
- interrupt
- later: MMU and page fault

既存の `os.asm` と `os.bin` は、すぐに動かす対象ではなく、将来の到達例として参照する。必要な概念は現在仕様に合わせて小さく作り直す。

Day 15 `POP` 以降の短期的な命令追加順は、ブログ記事とのつながりを優先して [manual/NEXT_INSTRUCTION_GUIDELINES.md](manual/NEXT_INSTRUCTION_GUIDELINES.md) にまとめる。

## Milestones

### 1. CPU v1

現在の `notes/vm.c` を、学習用の統合VMとして育てる。

必要な機能:

- 1MB の byte-addressed memory
- 32bit big-endian instruction fetch
- `PC = 0x00000000`
- register file
- condition flagsは段階的に追加する。現在は `CMP` / `JZ` / `JNZ` 用の `zero_flag` のみ
- stack with `PUSH` / `POP`
- basic instruction dispatch

命令は、ブログ記事で説明しやすい順に1つずつ追加する。

- `MOVI`
- `MOV`
- `ADD`
- `SUB`
- `CMP`
- `SBTI`
- `LDB`
- `STB`
- `LDDI`
- `STDI`
- `INC`
- `DEC`
- `PUSH`
- `POP`
- `CALLI`
- `RET`
- `JPI` / `JUMP`
- `JPZI` / `JZ`
- `JPNZI` / `JNZ`
- `JPUI`
- `JPNUI`
- `SYSCALL`
- `HALT`

外部バイナリローダーは、C配列に命令を直接置く段階から、`./vm program.bin` で実行する段階へ進むための節目として追加する。

### 2. OS v1

今回のVM仕様に合わせて、小さな自作OSを一から書く。

成功条件:

- 自作OSの起動メッセージが表示される
- `>` プロンプトが表示される
- 1文字入力を受け取れる
- 入力行をメモリに保存できる
- 最初の組み込みコマンドが動く

この時点で次の構成が成立する。

```text
自作CPU VM
  -> 自作OS
  -> OS内蔵シェル風コマンドループ
```

### 3. Shell v1

最初から C の shell をそのまま移植しない。

まずは自作OS内の小さなコマンドループを shell として育てる。

追加候補:

- `clear`
- `cat`
- `hexdump`
- `edit`
- `run`
- `help`

後で、自作した shell を `shell.bin` として OS から分離する。

```text
os.bin
  -> shell.bin
```

### 4. Executable ABI

OS とユーザープログラムの境界を決める。

初期案:

- program entry: logical address `0x00000`
- return: `RET` or `SYSCALL exit`
- `R0`: syscall argument 1
- `R1`: syscall argument 2
- `R0`: syscall return value
- `SP`: user stack
- caller-saved: `R0`-`R7`

必要に応じて後で拡張する。

### 5. Editor v1

いきなり kilo 全体を移植しない。

まずは小さな line editor または screen editor を作る。

最低限必要な syscall:

- print char
- print string
- read key
- clear screen
- move cursor
- open file
- read file
- write file
- close file
- exit process

最初の成功条件:

```text
> edit memo.txt
```

でエディター画面が開き、文字入力、保存、終了ができる。

### 6. C-like Application Layer

`Write a Shell in C` や `kilo` は、設計の考え方を学ぶために読む。

ただし、それらのCソースを今回のOSへそのまま移植することは当面の目標にしない。必要な処理だけを理解し、自作アセンブリまたは小さな専用言語で作り直す。

C ソースをそのまま自作CPUで動かすには、次が必要になる。

- 自作CPU向けコンパイラ、または既存コンパイラの backend
- assembler
- linker
- libc subset
- crt0
- syscall wrapper

これは後半の大きなテーマとして扱う。

当面は C の設計を読み、必要な処理を自作アセンブリまたは小さな専用言語で作る。

## First Target

現在の最初の実行目標:

```text
handmade-vm-roadmap/
  notes/
    vm.c
```

実行方法:

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm
```

現在の期待出力:

```text
A
VM flow complete.
CPU halted.
```

次の節目は、組み込みテストプログラムだけでなく、外部バイナリを読み込んで実行できる形にすること。

## Implementation Notes

既存教材から考え方を参考にしやすいもの:

- terminal raw mode
- Ctrl-C cleanup
- keyboard polling
- main execution loop structure
- optional trace output

今回の企画で一から作るもの:

- memory model
- register file
- instruction decoder
- syscall implementation
- binary loader
- interrupt model
- later: MMU

## Recommended Repository Shape

```text
handmade-vm/
  README.md
  ROADMAP.md
  src/
    mycpu.c
  os/
    os.asm
    shell.asm
  programs/
    pi.asm
    editor.asm
  tools/
    asmx.py
```

既存教材ディレクトリを直接使うのではなく、このリポジトリの中で小さく設計し、実装し、確認する。

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

ベースにする既存教材とコード:

- CPU/VM: `/Users/noi/wp/LC-3VM/lc3.c`
- OS: `/Users/noi/wp/os_dev/os_book_code/*/os.asm`
- Shell: `Tutorial - Write a Shell in C`
- Editor: `kilo`

ただし、LC-3 VM の仕様をそのまま使うのではなく、`os_book_code` の `emu.py` が実装している独自CPU仕様を C に移植する方針を取る。

## Direction

`LC-3VM/lc3.c` は「LC-3互換VM」として守るのではなく、CでVMを書くための出発点として扱う。

学習用VMの現在仕様は `notes/000-machine-state.md` を基準にする。

- 1MB memory
- 32bit instruction
- 20bit address space
- `R0`-`R7`, `SP`, `PC`
- `SYSCALL`
- interrupt
- later: MMU and page fault

既存の `os.asm` と `os.bin` は参考にするが、必要なら現在仕様に合わせて段階的に移植する。

## Milestones

### 1. CPU v1

`emu.py` の最小機能を C に移植する。

最初の対象は lesson 04。

必要な機能:

- 1MB の byte-addressed memory
- 32bit big-endian instruction fetch
- `PC = 0x00000000`
- `os.bin` の raw binary load
- register file
- condition flagsは未定。必要になった時点で仕様を追加する
- stack with `PUSH` / `POP`
- basic instruction dispatch

lesson 04 起動に必要な命令から実装する。

- `MOVI`
- `MOV`
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
- `JPI`
- `JPZI`
- `JPNZI`
- `JPUI`
- `JPNUI`
- `SYSCALL`
- `HALT`

### 2. OS v1

`lessons/04-simple-os-commands/os.asm` を起動する。

成功条件:

- `Welcome to Simple OS!` が表示される
- `>` プロンプトが表示される
- `reg` が動く
- `ls` が動く
- `date` が動く
- `exec pi.bin` が動く

この時点で次の構成が成立する。

```text
自作CPU VM
  -> 自作OS
  -> OS内蔵シェル風コマンドループ
```

### 3. Shell v1

最初から C の shell をそのまま移植しない。

まずは `os.asm` 内の `cmdloop` を shell として育てる。

追加候補:

- `clear`
- `cat`
- `hexdump`
- `edit`
- `run`
- `help`

後で shell を `shell.bin` として OS から分離する。

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

`Write a Shell in C` や `kilo` を参考に、自作OS用に移植する。

C ソースをそのまま自作CPUで動かすには、次が必要になる。

- 自作CPU向けコンパイラ、または既存コンパイラの backend
- assembler
- linker
- libc subset
- crt0
- syscall wrapper

これは後半の大きなテーマとして扱う。

当面は C の設計を読み、必要な処理を自作アセンブリまたは小さな専用言語で移植する。

## First Target

最初に作るもの:

```text
/Users/noi/wp/handmade-vm-roadmap/
  ROADMAP.md

/Users/noi/wp/LC-3VM/
  mycpu.c
```

最初の実行目標:

```sh
cd /Users/noi/wp/os_dev/os_book_code/lessons/04-simple-os-commands
python3 asmx.py os.asm

cd /Users/noi/wp/LC-3VM
cc mycpu.c -o mycpu
./mycpu /Users/noi/wp/os_dev/os_book_code/lessons/04-simple-os-commands/os.bin
```

期待する最初の表示:

```text
Welcome to Simple OS!
>
```

## Implementation Notes

`LC-3VM/lc3.c` から流用しやすいもの:

- terminal raw mode
- Ctrl-C cleanup
- keyboard polling
- main execution loop structure
- optional trace output

作り直すもの:

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

既存教材ディレクトリを直接大きく壊さず、新しい実験用ディレクトリで統合していく。

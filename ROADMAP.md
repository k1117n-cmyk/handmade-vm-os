# Handmade VM Roadmap

最終整理: 2026-10-20。実装の現在地はDay 35。この技術ロードマップでは、記事番号と実装段階を区別し、実装済みの機能と今後の設計候補を分けて記録する。

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
- アドレス値・PC・SPは32bit。使用できるメモリ範囲は `0x00000..0xFFFFF`、命令の即値フィールドは20bit
- `R0`-`R7`, `SP`, `PC`
- `SYSCALL 0`: 1文字と改行を表示
- `SYSCALL 1`: 0終端文字列を表示
- `SYSCALL 2`: host標準入力から1 byteを読む
- `SYSCALL 3`: 1文字を改行なしで表示

割り込み、MMU、ページフォールトは未実装の長期候補。現在のコマンドループを動かす前提にはしない。

既存の `os.asm` と `os.bin` は、すぐに動かす対象ではなく、将来の到達例として参照する。必要な概念は現在仕様に合わせて小さく作り直す。

基本命令を追加した順序と今後の判断方針は、ブログ記事とのつながりを含めて [manual/NEXT_INSTRUCTION_GUIDELINES.md](manual/NEXT_INSTRUCTION_GUIDELINES.md) にまとめる。

外部バイナリの読み込みと入出力は実装済み。現在はDay 35の `programs/word-command-loop.bin` で、help / unknownの後に再入力し、quitで終了するところまで進んでいる。

次の候補は、小さな組み込みコマンドの追加。echoを題材に、入力と出力を仕様カードで決める。ブログの説明順と中期候補は `articles/ROADMAP.md`、作業再開点は [manual/HANDOFF.md](manual/HANDOFF.md) を参照する。

## Milestones

### 1. CPU / VMの基礎（実装済み）

現在の `notes/vm.c` を、学習用の統合VMとして育てる。

現在の機能:

- 1MB の byte-addressed memory
- 32bit big-endian instruction fetch
- `PC = 0x00000000`
- register file
- condition flagsは段階的に追加する。現在は `CMP` / `JZ` / `JNZ` 用の `zero_flag` のみ
- stack with `PUSH` / `POP`
- basic instruction dispatch

命令は、ブログ記事で説明しやすい順に1つずつ追加する。現在は、次の基本命令まで進んでいる。

- `MOVI`
- `MOV`
- `ADD`
- `SUB`
- `CMP`
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
- `JUMP`
- `JZ`
- `JNZ`
- `SYSCALL`
- `HALT`

外部バイナリローダーは、C配列に命令を直接置く段階から、`./handmade-vm programs/hello.bin` で実行する段階へ進むための節目として追加した。

VM・host側ツール・VM上のサンプルで完了した節目:

```text
1. 外部バイナリローダー
2. 小さな hello.bin の作成方法
3. hello.bin 版の最小デモ
4. small assembler
5. 入力系 SYSCALL
6. プロンプト付き1文字echo
7. h / q / その他の1文字コマンド判定
8. q まで待ち続ける1文字コマンドループ
9. 起動メッセージとhelp表示
10. Enterまでの入力をVM内メモリへ保存する入力バッファ
11. 引数なし起動で外部バイナリを選べるhost側monitor mode
12. 入力バッファの help / quit 文字列比較
13. 入力バッファ処理の CALLI / RET サブルーチン化
14. 文字列比較の共通サブルーチン化
15. 複数文字コマンドループと、上限後・CRLFの残り入力の処理
```

現在の hello 実行:

```text
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

`programs/hello.bin` の中身:

```asm
MOVI R0, 65
SYSCALL 0
HALT
```

期待出力:

```text
A
CPU halted.
```

現在の `tools/small-asm.c` は `MOVI`, `SYSCALL`, `HALT`, `CMP`, `JUMP`, `JZ`, `JNZ` を扱う。Day 35などのサンプルは専用writerで生成する。今後は必要に応じて、命令対応、ラベル解決、文字列データ配置を別々の課題として拡張する。

Day 35までで、入力・比較サブルーチンを使った複数文字コマンドループが完成した。保存上限は63 byteで、上限後もLFまで消費する。CRは無視し、CR単独では入力を確定しない。

命令名の `JPI` / `JPZI` / `JPNZI` は参考教材側の呼び名で、現在のJUMP / JZ / JNZに相当する。small-asmが扱う表記とは区別する。

今後必要になれば、比較フラグやビット操作などを追加する。参考教材の `SBT` / `SBTI` / `NOT` / `JPUI` / `JPNUI` をそのまま実装予定とはせず、現在のISAで必要な意味・命令形式を仕様カードで定義する。`underflow_flag` は命令ではなく条件フラグの候補として扱う。

### 2. VM上のコマンドループからOSのサービスへ

OS風の入口として、起動メッセージ、1文字入力、行入力、組み込みコマンドは実装済み。Day 35ではhelp / quitを使う行単位のループまで進んだ。

```text
host側monitor（実装済み）
  ファイル名を読み、hostファイルを開く
  新しいVMへ.binを読み込んで実行する

VM上のプログラム（実装済み）
  表示と入力のsyscallを使う
  入力バッファを比較し、組み込みコマンドを実行する
```

次にOSとして何を受け持つかを、小さなサービスごとに設計する。現在はVM上からファイルを開く機能、別の実行プログラムを読み込む機能、プロセス管理はない。

候補となる節目:

- 入出力やバッファ処理を、呼び出しの約束とともに整理する。
- 保存先をhostファイルにするか仮想ディスクにするかを決める。
- 読み書きの成功・失敗をVM側へ返す仕組みを作る。
- 別プログラムをロードし、終了後に戻る仕組みを設計する。

hostファイル用syscallを追加する段階と、VM上に自作ファイルシステムを作る段階は分ける。OSの機能が必要になる前に、組み込みコマンドやメモリ上の行編集を進めてもよい。

### 3. Shell v1（組み込みコマンドから育てる）

現在のhelp / quit / unknownを出発点にする。次の第一候補はechoなどのコマンド追加で、入力と出力の具体例から仕様を決める。

短期の順序案:

```text
コマンドを1つ追加する
  → 必要に応じてsmall-asmを拡張する
  → コマンド名と引数を分ける
  → メモリ上のデータを扱うコマンドへ進む
```

コマンド候補と前提:

| 候補 | 必要になるもの |
|---|---|
| echo | 表示対象の入力方法、バッファの再利用規則 |
| hexdump | 最初はVM内メモリの範囲指定と16進表示。ファイルを対象にするなら読み込み機能 |
| edit | 編集用バッファと操作。最初はメモリ上の行編集 |
| cat | ファイルまたは仮想ディスクの読み込み機能 |
| clear | 端末表示の制御方法と、そのVM側インターフェース |
| run | ローダーと起動・終了・復帰の約束 |

将来、shellを `shell.bin` としてOSから分離する。これは未実装であり、先に次の実行規約を決める必要がある。

### 4. Executable ABI（設計候補・未確定）

OSとユーザープログラムの境界を決める。現在はhost側ローダーが新しいVMのmemory[0]へ読み込み、PC=0から実行する。VM上のOSから別プログラムを呼び出す規約はまだない。

設計時に決める項目:

- ロード先と入口アドレス。OSやshellのメモリを上書きしない配置。
- PC・SP・レジスタを起動時にどう設定し、復帰時に何を戻すか。
- 引数、戻り値、エラーを渡す場所。
- 呼び出し側と呼び出される側のどちらが各レジスタを保存するか。
- プログラムの終了をRETにするか、専用の終了syscallにするか。

同一VMへロードする場合、入口を単純に0へ固定すると常駐するコードと衝突し得る。配置や退避の設計が必要。現在MMUはないため、論理アドレス空間が分離されているとは扱わない。

RETを終了方法にする場合は、呼び出し側が有効な戻り先をスタックへ積む必要がある。HALTは現在VMを停止する命令で、VM上のshellへ戻る機能ではない。新しい終了syscallの番号や動作は未定。

read_lineやstring_equalのレジスタ利用は、現在のサンプル内の約束。すべてのプログラムに適用するABIとして確定したものではない。

### 5. Editor v1（段階的に追加）

既存エディターの設計を参考にしながら、まず小さな行編集を一から作る。

| 段階 | 成功条件 | 必要なもの |
|---|---|---|
| メモリ上の1行編集 | 入力した行を保持し、表示・置換できる | 現在の行入力・文字列表示、編集用バッファ |
| 複数行の編集 | 行を選び、追加・置換・削除できる | 行のデータ構造、容量上限、行指定 |
| 保存と読み込み | 編集結果を保存して読み戻せる | ファイルまたは仮想ディスクへの入出力とエラー処理 |
| shellから起動・復帰 | editで入り、終了後にshellへ戻る | プログラムを分ける場合はローダーと実行規約 |
| 画面全体を使う編集 | キー操作、カーソル移動、再描画ができる | キー単位の入力、端末制御、画面状態の管理 |

最初の行編集は現在のsyscallを使って始められる。raw mode、clear screen、move cursor、open / read / write / close、exitなどをすべて先に揃える必要はない。追加するインターフェースは各段階の仕様カードで決める。

将来の到達例:

```text
> edit memo.txt
```

で編集を始め、保存してshellへ戻る。これは最初の成功条件ではなく、保存と起動・復帰まで整えた後の目標とする。

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

## Current Target

現在のサンプルを実行する:

```sh
make run-word-command-loop
```

Command>へhelp、hello、quitを順に入力する。自動照合は `make test-word-command-loop`（Python 3、26ケース）。

次はechoなどの追加コマンドの仕様カードを書く。実装の前に、入力形式、表示内容、既存ループへの戻り方を決める。

現在の基本実行:

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

引数なしではhost側monitorが起動する。

```text
Welcome to Handmade VM
>
```

内蔵テストは `--self-test` で実行する。

```sh
/tmp/handmade-vm --self-test
```

```text
A
VM flow complete.
CPU halted.
```

外部バイナリの実行:

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

期待出力:

```text
A
CPU halted.
```

`programs/hello.bin` は、`tools/write-hello-bin.c` または `tools/small-asm.c` で再生成できる。

Day 24では、`SYSCALL 2` を「host標準入力から1 byte読み、`R0`へ入れる」機能として追加した。

Day 25では、`SYSCALL 2` を使った最小echoプログラム `programs/echo-char.asm` と `programs/echo-char.bin` を追加した。

Day 26では、新しい命令を追加せず、`>` を表示してから1文字echoする `programs/prompt-echo.asm` と `programs/prompt-echo.bin` を追加した。

Day 35では、入力・比較サブルーチンを使い、help / unknown後に再入力し、quitで終了するループを追加した。次の作業候補はechoなどのコマンド追加で、具体的な動作は仕様カードで決める。

短い再開手順は [manual/HANDOFF.md](manual/HANDOFF.md) を見る。

## Implementation Notes

既存教材から考え方を参考にしやすいもの:

- terminal raw mode
- Ctrl-C cleanup
- keyboard polling
- main execution loop structure
- optional trace output

実装済みで、今後も自作して拡張するもの:

- memory model
- register file
- instruction decoder
- syscall implementation
- binary loader

必要になった段階で設計する長期候補:

- interrupt model
- MMU、ページング、メモリ保護

## Recommended Repository Shape

```text
handmade-vm-roadmap/
  README.md
  ROADMAP.md
  manual/
    specs/
    test-code-explanations/
  notes/
    vm.c
    001-halt-test.c
    ...
  programs/
    hello.asm
    hello.bin
    later: os.bin
    later: pi.asm
    later: editor.asm
  os/
    later: os.asm
    later: shell.asm
  tools/
    write-hello-bin.c
    small-asm.c
```

既存教材ディレクトリを直接使うのではなく、このリポジトリの中で小さく設計し、実装し、確認する。

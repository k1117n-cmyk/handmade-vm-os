# Handmade VM Roadmap

最終整理: 2026-11-20。実装の現在地はDay 45。この技術ロードマップでは、記事番号と実装段階を区別し、実装済みの機能と今後の設計候補を分けて記録する。

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
- `SYSCALL 4` / `5`: 固定名line.txtへ1行を保存・読み込み
- `SYSCALL 6`: 固定のrun-child.binを独立した子VMで同期実行
- `SYSCALL 8`: 固定のeditor.binを独立した子VMで同期実行（7は未実装）

未実装の長期候補は[Implementation Notesの一覧](#未実装の長期候補)を参照する。

既存の `os.asm` と `os.bin` は、すぐに動かす対象ではなく、将来の到達例として参照する。必要な概念は現在仕様に合わせて小さく作り直す。

基本命令を追加した順序と今後の判断方針は、ブログ記事とのつながりを含めて [manual/NEXT_INSTRUCTION_GUIDELINES.md](manual/NEXT_INSTRUCTION_GUIDELINES.md) にまとめる。

外部バイナリの読み込みと入出力に加え、Day 43で固定名line.txtへの保存・読み込み、Day 44で別プログラムの同期起動と復帰を実装した。Day 45では `programs/shell.bin` と `programs/editor.bin` を分け、shellのeditからeditorを起動して、終了後にshellへ戻るところまで進んでいる。

editorはset / show / save / loadを持ち、本文は最大63 byte。毎回空で起動し、復元には明示loadを使う。quitとEOFでは自動保存・確認質問を行わない。保存はline.txtへ直接上書きし、保存途中の失敗では旧ファイル保持を保証しない。load失敗時は編集内容を保持する。

2026-11-20時点で次の機能は未着手。仕様案の作成まではHANDOFFの段取りに従って進め、仕様の確定と実装への着手はユーザーの指定に従う。仮想ディスク、任意ファイル名、複数行、画面制御、引用符の解釈、複数引数は後に回す。ブログの説明順と中期候補は `articles/ROADMAP.md`、作業再開点は [manual/HANDOFF.md](manual/HANDOFF.md) を参照する。

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
16. echoコマンドと、small-asmの命令・ラベル・文字列対応、echoの.asm移行（Day 36〜40）
17. コマンド名と本文の分割（Day 41）
18. 入力とは別の編集バッファで1行を保持・置換・表示（Day 42）
19. 固定名line.txtへの明示保存・読み込み（Day 43）
20. 固定の子プログラムを独立したVM状態で同期起動し、親へ復帰（Day 44）
21. shell / editorの分離と、editによる起動・復帰（Day 45）
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

Day 37で `tools/small-asm.c` は既存7命令に `LDB`, `STB`, `INC`, `CALLI`, `RET` を加え、12命令に対応した。Day 36のechoなど、文字列と配置が必要なサンプルは専用writerで生成する。Day 38で固定長命令列のラベル解決（分岐・CALLI、前方参照、重複・未定義の検査）まで完了した。Day 39でASCIIの0終端文字列、.align 4、MOVIのラベル参照を追加し、最小表示例まで確認した。Day 40でechoの.asm移行を完了。70命令と8文字列を421 byteへ詰めて配置し、固定バッファ0x500との非重複と既存echoの54ケースを確認した。絶対配置指示の追加は不要だった。Day 41でコマンド名と本文の分割を追加し、分割14ケース・入出力74ケースを確認した。Day 42で入力0x500と編集0x600を分け、set / showによる保持・置換・表示を実装した。部品8ケース、入出力75ケースを確認。

Day 43では固定名line.txtへ本文0〜63 byteを保存・読み込みするSYSCALL 4/5を追加した。Day 44では固定のrun-child.binを独立した子VMで同期実行するSYSCALL 6を追加した。Day 45ではshell / editorを分け、SYSCALL 8でeditor.binを起動する。ロード14・VM経由20・入出力265ケースを確認済み。shellは827 byte、editorは965 byteで、ともに入力領域0x500より手前へ配置する。[保存の仕様](manual/specs/043-line-storage.md)、[起動の仕様](manual/specs/044-program-launch.md)、[分離の仕様](manual/specs/045-shell-editor.md)を参照する。

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
  表示・入力・固定名ファイルの保存/読み込みsyscallを使う
  shellが組み込みコマンドを処理し、run/editで独立した子VMを同期起動する
  editorで1行を編集し、終了後にshellのコマンド入力へ戻る
```

Day 43〜45のサービスはhost側のsyscall実装が提供する。固定名line.txtの読み書きと、固定の2プログラムを独立した子VMで同期実行する機能は実装済み。親のメモリ・スタックを保持し、正常HALTならR0=0、起動失敗や異常停止ならR0=1で戻る。

VM上に常駐するOS、仮想ディスク・自作ファイルシステム、任意パスの起動、汎用的なプロセス管理は未実装。今後必要になった段階で、現在のhost側サービスとVM上のOSが受け持つ範囲を小さく設計する。

hostファイル用syscallを追加する段階と、VM上に自作ファイルシステムを作る段階は分ける。

### 3. Shell v1（組み込みコマンドから育てる）

Day 36のecho、Day 41の本文分割、Day 42〜43の行編集・保存を経て、Day 45で独立した `programs/shell.asm` / `.bin` を追加した。

| 実装済みコマンド | 動作 |
|---|---|
| help | shellが受け付けるコマンドを表示 |
| echo [text] | 同じ行の本文を表示。単独ならText>で次の1行を読む |
| run | 固定run-child.binをSYSCALL 6で同期実行 |
| edit | 固定editor.binをSYSCALL 8で同期実行し、終了後にCommand>へ戻る |
| quit | shellを終了。host側monitorから起動した場合はmonitorへ戻る |

set / show / save / loadはeditorの責務であり、shellでは受理しない。旧program-launcherは従来のコマンドを保った別サンプルとして残す。

今後の候補と前提:

| 候補 | 必要になるもの |
|---|---|
| hexdump | 最初はVM内メモリの範囲指定と16進表示。ファイルを対象にするなら読み込み機能の拡張 |
| cat | 表示対象の選択と、ファイル読み込み・失敗時の規約 |
| clear | 端末表示の制御方法と、そのVM側インターフェース |

shell.binはVM上の独立プログラムとして実装済み。将来の常駐OSからshellを起動する構成は未実装であり、次の汎用実行規約とは区別する。

### 4. Executable ABI（固定起動は実装済み・汎用ABIは設計候補）

Day 44〜45で固定の子を起動する最小規約を実装した。SYSCALL 6/8はhost側で独立した子VMを初期化し、memory[0]へ読み込んでPC=0、SP=0x100000、R0〜R7=0、zero flag=falseで同期実行する。子の正常HALTで親R0へ0、それ以外の停止やロード失敗で1を返し、親の他レジスタ・メモリ・スタック・フラグを保持してsyscallの次へ戻る。

標準入出力と作業ディレクトリは共有する。子からのSYSCALL 6/8は起動せず失敗1を返す。任意パス・引数・任意の終了値は渡せない。

VM上の常駐OSとユーザープログラムの汎用ABIは未確定。以下は同一VMへの配置や汎用的な呼び出しを設計する場合の候補であり、現在の固定起動に必要な未決事項ではない。

設計時に決める項目:

- ロード先と入口アドレス。OSやshellのメモリを上書きしない配置。
- PC・SP・レジスタを起動時にどう設定し、復帰時に何を戻すか。
- 引数、戻り値、エラーを渡す場所。
- 呼び出し側と呼び出される側のどちらが各レジスタを保存するか。
- プログラムの終了をRETにするか、専用の終了syscallにするか。

現在の子VMは親とは別のメモリ配列を持つ。これはMMUによるアドレス空間の分離ではない。同一VMへロードする場合、入口を単純に0へ固定すると常駐するコードと衝突し得るため、別途配置や退避の設計が必要。

RETを終了方法にする場合は、呼び出し側が有効な戻り先をスタックへ積む必要がある。現在のHALTは実行中のVMを停止する命令であり、子VMの停止後にhost側の起動処理が親shellへ結果を返す。同一VM内のOSへ戻る終了syscallの番号や動作は未定。

read_lineやstring_equalのレジスタ利用は、現在のサンプル内の約束。すべてのプログラムに適用するABIとして確定したものではない。

### 5. Editor v1（段階的に追加）

既存エディターの設計を参考にしながら、1行の保持・置換から段階的に拡張する。現在は保存・読み込みとshellからの起動・復帰まで実装済み。

| 段階 | 状態 | 成功条件・必要なもの |
|---|---|---|
| メモリ上の1行編集 | Day 42で実装済み | 入力とは別のバッファで1行を保持・表示・置換 |
| 保存と読み込み | Day 43で実装済み | 固定line.txtへ本文0〜63 byteを明示save/load。失敗を通知 |
| shellから起動・復帰 | Day 45で実装済み | editで独立したeditorへ入り、quitでshellへ戻る |
| 複数行の編集 | 後に回す | 行のデータ構造、容量上限、行指定・追加・置換・削除の規約 |
| 画面全体を使う編集 | 後に回す | キー単位の入力、端末制御、カーソルと画面状態の管理 |

最初の行編集は現在のsyscallを使って始められる。raw mode、clear screen、move cursor、open / read / write / close、exitなどをすべて先に揃える必要はない。追加するインターフェースは各段階の仕様カードで決める。

将来の到達例:

```text
> edit memo.txt
```

で指定したファイルの編集を始め、保存してshellへ戻る。現在のeditは引数を受け付けず、保存先も固定line.txtである。任意ファイル名の指定は後の目標とする。

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

```sh
make run-shell
```

Command>へeditを入力し、Editor>でset hello、show、save、quitを順に入力する。
shellへ戻ったら再びedit、show（空内容）、load、show（hello）、quitと入力し、最後にshellのquitで終了する。
editorは毎回空で起動するため、内容を残すには明示saveが必要。

自動照合は `make test-shell-editor-parts test-shell-editor`（ロード14・VM経由20・入出力265ケース）。
[Day 45仕様](manual/specs/045-shell-editor.md)と[解説](manual/test-code-explanations/045-shell-editor.md)を参照。
2026-11-20時点で、Day 45の実装・検証は完了済みで、次の機能は未着手。

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

Day 35では、入力・比較サブルーチンを使い、help / unknown後に再入力し、quitで終了するループを追加した。その後、echo、本文分割、行編集、保存・読み込み、別プログラム起動を経て、Day 45のshell / editor分離まで進んだ。

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

### 未実装の長期候補

次の機能は未実装で、現在のコマンドループを動かす前提にはしない。必要になった段階で設計する。

- 割り込み（interrupt model）
- MMU、ページング、ページフォールト、メモリ保護

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
    shell.asm
    shell.bin
    editor.asm
    editor.bin
  os/
    later: os.asm
  tools/
    write-hello-bin.c
    small-asm.c
```

既存教材ディレクトリを直接使うのではなく、このリポジトリの中で小さく設計し、実装し、確認する。

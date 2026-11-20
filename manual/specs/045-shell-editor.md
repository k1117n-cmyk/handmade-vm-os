# Day 45: shell / editorの分離（承認済み）

## 状態と今回の完了条件

2026-11-20、ユーザーが会話で推奨案を承認し、そのまま進めるよう指示した。
以下は承認済みの仕様。今回の対象は部品確認、VM統合、新shell/editorの生成・入出力検証と文書更新。
完了条件は下記の規約・回帰セット・生成物の約束を実物で確認すること。記事作成・公開・コミットは行わない。

## 調査した現状と引き継ぐ規約

基準は[機械状態](000-machine-state.md)、[命令形式](../instruction-types.md)、
[field](../reference/instruction-fields.md)、[行編集](042-line-editor.md)、
[保存・読み込み](043-line-storage.md)、[別プログラム起動](044-program-launch.md)。

- `programs/program-launcher.asm`は同じループでhelp / echo / set / show / save / load / run / quitを処理する。
  `do_run`だけがSYSCALL 6を呼び、編集内容は親の0x600にある。
- `notes/program-launch.h`のload_child_programは`programs/run-child.bin`を固定名で開く。
  `notes/vm.c`のlaunch_programは独立した子VMを初期化し、同期実行後に解放する。
  現在のSYSCALL 6にはファイル名・起動対象を選ぶ入力がない。
- 子VMは起動ごとに1 MiBメモリとR0〜R7をゼロ初期化し、PC=0、SP=0x100000、zero flag=falseで開始する。
  HALTなら親R0へ0、それ以外の停止や起動失敗なら1。親の他レジスタ・全メモリ・SP・フラグを保持して次の命令へ戻る。
  標準入出力と作業ディレクトリを共有し、子からのSYSCALL 6は起動せず失敗1で続行する。
- 保存・読み込みはSYSCALL 4/5、固定名`line.txt`、本文0〜63 byte。成功0・失敗1を返す。
  読み込み失敗では編集メモリを保持し、直接上書き保存の途中失敗では旧ファイル保持を保証しない。
- 既存の入力上限63 byte、LF確定・CR無視、超過分のLFまでの消費、最初のNULまでの処理、
  UTF-8のbyte扱い、半角空白による分割、set単独と区切りだけの違いを引き継ぐ。
- 既存サンプルとSYSCALL 0〜6は従来の約束を維持する。Day 42で独立editorを対象外としたのは旧サンプルの範囲であり、
  旧サンプルを置換せず新サンプルで分離を学ぶ。

## 承認した仕様と比較案

下表の推奨案をすべて採用した。最小仕様にユーザー判断を要する未決事項はない。

| 判断する事項 | 推奨案 | 別案と推奨理由 |
|---|---|---|
| 分離する責務 | 新shellがhelp / echo / run / edit / quit、別editorがhelp / set / show / save / load / quit | shellにも編集機能を残す案では責務が重なる。旧program-launcherは残して互換性を保つ |
| 起動入口 | `edit`が固定の`programs/editor.bin`を新SYSCALL 8で同期起動 | 6の起動先変更はDay 44の約束を破る。汎用パス渡しは任意ファイル名を扱う規約が増える |
| syscall番号 | 8（命令byte `60 00 00 08`）。7は未実装のまま残す | 7を使う案もあるが、既存Day 44テストは7を未実装syscallの診断と失敗検出に使う。8なら既存の負の確認と期待値をそのまま残せる |
| プログラム名・プロンプト | `programs/shell.asm` / `.bin`は`Command> `、`programs/editor.asm` / `.bin`は`Editor> ` | 両方Command>なら現在の操作対象が見分けにくい |
| 編集内容の寿命 | editorの子メモリだけに保持。起動時は空、終了で破棄。残すには明示save、次回は明示load | 自動load/saveや親へのメモリ返却は起動・終了の約束を増やす。明示操作なら既存保存規約を利用できる |
| 保存先 | 同じ作業ディレクトリの固定名`line.txt`を継承 | 別名・複数ファイル・仮想ディスクは今回導入しない |
| 終了・失敗通知 | editorのquitは`Goodbye from editor`とLF、既存HALTを実行。shellは結果0なら`Editor finished.`、1なら`Editor failed.`とLFを表示して再入力 | 終了結果だけで保存成否を表す案は採らず、保存成否はeditor内の既存通知で分かるようにする |

## 操作と起動・復帰

### shell

起動時から`Command> `で待つ。編集バッファは持たない。

| コマンド | 動作 |
|---|---|
| `help` | 下記shell helpを表示 |
| `echo [text]` | Day 41と同じ。単独なら`Text> `で次の行、区切りだけなら空本文を表示 |
| `run` | 既存SYSCALL 6でrun-child.binを実行。Day 44の`Run finished.` / `Run failed.`を継承 |
| `edit` | SYSCALL 8でeditor.binを実行。結果通知後にCommand>へ戻る |
| `quit` | 既存の`Goodbye from Handmade VM`とLFを表示してHALT。monitorから起動した場合はmonitorへ戻る |
| その他 | `Unknown command`とLF。set / show / save / loadはshellでは受理しない |

help / run / edit / quitの非空引数を拒否。先頭・末尾の半角空白だけは受理する。
大小文字・タブ・空行・NUL・途中EOFなどは既存の入力規約を引き継ぐ。
help本文は各行LF付きで次の通り。

```text
Commands:
help: show help
echo [text]: print text or read a line
run: start run-child.bin
edit: start editor.bin
quit: quit
```

### editor

新しい子VMで起動し、編集バッファを空にして`Editor> `で待つ。自動loadは行わない。
help / set / show / save / load / quitを受理し、echo / run / editや未知のコマンドは拒否する。
set / showはDay 42、save / loadはDay 43を継承し、コマンド後の戻り先だけEditor>とする。
set単独ならText>で次の行を読み、区切りだけのsetは空内容にする。
同じ行の本文最大59 byte、次の行なら最大63 byte。短縮・空への置換で古い末尾を表示しない。
help / show / save / load / quitの非空引数は拒否し、半角空白だけの末尾は許す。
help本文は各行LF付きで次の通り。

```text
Commands:
help: show help
set [text]: replace the stored line or read a line
show: print the stored line
save: write the stored line to line.txt
load: read the stored line from line.txt
quit: return to shell
```

save / loadの通知は`Saved.` / `Save failed.` / `Loaded.` / `Load failed.`とLF。
保存・読み込みの失敗でもeditorを停止せずEditor>へ戻る。
quitとEOFでは自動保存・確認質問を行わず、未保存内容は失われる。
標準入力EOFでは既存の`input EOF`で子が異常停止し、shellはEditor failed.を表示して再入力する。
同じ入力がEOFのままなら、続いてshellも既存のEOF処理で停止する。EOFを解除する処理は追加しない。

### SYSCALL 8の規約

入力レジスタなし。親VMだけが固定名`programs/editor.bin`を起動できる。
ロード容量4 byte〜1 MiB、初期状態、同期実行、失敗判定、親R0への0/1、親状態の保持、資源解放はSYSCALL 6と同じ。
R0以外を変えず、fetchで進んだPCから親が再開する。
子VMからSYSCALL 6または8を呼んだ場合はファイルを開かずR0=1でその子へ戻り、多重起動しない。
子の正常HALTは`CPU halted.`を表示し、その後にshellの結果通知が出る。
子の実行エラー診断もshellの結果通知より先に出る。
結果0は正常HALTを意味し、編集内容が保存済みであることは意味しない。
時間制限は追加せず、子が停止しなければ親は待つ。

## メモリ・保存・生成物の約束

- shellとeditorは独立したメモリ空間。双方とも命令・文字列末尾の次を0x500以下とし、入力を0x500〜0x53Fに置く。
- editorだけが0x600〜0x63Fを編集用に使用する。スタックは各VMの0x100000から下向き。
- shellへ編集メモリや終了値を渡す新機構は追加しない。line.txtだけを明示保存・読み込みで共有する。
- line.txtは本文のみ0〜63 byte、終端NUL・改行なし。load失敗時の保持とsave途中失敗時の制限はDay 43通り。
- 新shell/editorの命令数・binサイズは実装後に実測する。各asmから再生成したbinとの全byte一致を確認する。
- 既存program-launcher（1198 byte）、run-child（3命令12 byte）と旧サンプルは変更せず、開始時のbyteを保持する。
  1198 byteは現物サイズも確認すること。旧サンプルとの新bin全体一致は要求しない。

## 実装の変更範囲

| 対象 | 必要な変更 |
|---|---|
| このカード | 承認済み規約と部品の入口を記録してから実装開始 |
| notes/program-launch.h | 固定の2起動先を扱うロード部品を用意。既存load_child_programの入口・動作を保持し、共有処理へ渡すパスはhost内の定数に限定 |
| notes/vm.c | 既存launch_programの動作を保持し、editor用の同期起動とSYSCALL 8を追加。6/8で同じ子初期化・解放規約を使う |
| notes/045-shell-editor-test.c（新規） | editor用ロードと起動、親状態保持、失敗、多重起動拒否を部品・統合の両方で確認 |
| programs/shell.asm・.bin、programs/editor.asm・.bin（新規） | 責務を分けた2プログラム。既存asmは置換しない |
| tools/test-shell-editor.py（新規） | 固定期待値でstdout全文・stderr・終了コード・ファイル・配置・再生成一致を確認 |
| Makefile | 新規生成物と部品・入出力テスト、起動用ターゲットを追加 |
| README.md、programs/README.md、manual/README.md、manual/test-code-explanations/045-shell-editor.md（新規） | 実行手順と実測値・解説を実装検証後に記載 |
| manual/HANDOFF.md、manual/WORKLOG.md | 実装検証後に結果と次の段取りを記録 |

small-asm・writer・旧サンプル・既存テストの期待値は変更不要と見込む。
SYSCALL 8では既存test-program-launch.pyの未実装SYSCALL 7と失敗分岐用7をそのまま残せる。
manual/instruction-types.mdには既存のSYSCALL形式にimm=8が追加されたことを記録する（新しいtype/opは追加しない）。

## 実装の順序と完了条件

1. 承認済み仕様をカードへ反映し、部品の入口・保持する状態・期待結果を記録する。
2. editor用ロードを欠落、0〜3 byte、4 byte、1 MiB前後、open/read/close・末尾確認失敗で小さく確認する。
   既存SYSCALL 6の起動先が変わらないことも確認する。
3. 部品確認後にVMへ統合。HALT・不正命令・範囲外アクセス・EOF・確保失敗・ロード失敗を確認する。
   親の全メモリ・R1〜R7・SP・フラグ・running・復帰PC、R0=0/1、解放数を照合する。
   子から6/8を呼んでもファイルを開かず1で続行すること、起動ごとの初期化を確認する。
4. 新shell/editorを生成し、次をstdout全文・空stderr・終了コードとファイルbyteで照合する。
   - edit → set hello → show → save → quit → echo back → edit → show（空）→ load → show（hello）→ quit → quit。
   - 未保存終了と再起動時の空内容、起動時に自動loadしないこと、quit/EOFで自動saveしないこと。
   - load失敗時の編集内容保持、save失敗後の操作継続、空と63 byte、64 byteやNUL/CR/LFを含む保存ファイルの拒否。
   - editor欠落・短すぎるファイル・実行エラー後のshell継続、共有入力と途中EOF、1000回の起動・初期化・解放。
   - shellとeditorの責務外コマンド拒否、help全文、引数・空白・CRLF・上限・NUL・UTF-8・set単独の入力処理。
   - shellのrunが旧run-child.binを起動し、editがeditor.binを起動すること。monitorへの復帰と再起動。
5. 各binの再生成一致、命令数・文字列数・サイズ・配置、line.txtの本文完全一致、旧サンプルのbyte保持を確認する。
6. 以下のDay 44回帰セットを実行し、既存件数・期待値・配置を保持したうえでREADME・manual類へ実測を記録する。

```sh
make test-program-launch-parts
make test-program-launch
make test-line-storage-parts
make test-line-storage
make test-line-copy test-line-editor test-command-split test-command-arguments
make test-small-asm test-small-asm-labels test-small-asm-strings
make run-asm-string all test test-monitor test-echo-command test-word-command-loop test-asm-echo-command
./handmade-vm --self-test
cmp programs/asm-memory-call.bin programs/asm-label-call.bin
```

## 後に回す事項

仮想ディスク、任意ファイル名、複数行、画面制御、引用符の解釈、複数引数、
部分編集、終了時の保存確認、自動保存・自動読み込み、親子のメモリ受け渡し、任意終了値、
多重起動、並行実行、実行時間制限、記事作成・公開・コミット。

## 部品の実装規約と検証対応

ロード共通処理はnotes/program-launch.hのload_fixed_program(memory, capacity, path)とする。
呼出元はhost内の固定文字列だけを渡し、VMから任意パスは受け取らない。
load_child_program(memory, capacity)は従来のrun-child.bin、load_editor_program(memory, capacity)はeditor.binを選ぶ。
4 byte〜capacityのread・末尾確認・close成功時だけtrue。ロード部品は画面表示しない。

VMは内部のlaunch_loaded_program(loader)で子確保・初期化・is_child設定・ロード・run・解放を共有する。
既存launch_program(void)と新launch_editor(void)が固定ロード関数を選ぶ。親の状態変更はsyscall実行側のR0代入だけ。
notes/045-shell-editor-test.cはまずロードのみで実行し、統合後は-DLAUNCH_VM_TESTでも実行する。
stdio・malloc/freeの置換で故障と資源解放を確認し、親全メモリ・全レジスタ・SP・フラグ・復帰PCを比較する。
子のSYSCALL 6/8をそれぞれ検査し、ファイルopen・子確保を行わず失敗1で子が続行することを確認する。
copy_line / split_line / read_line / compare_commandは既存asmの規約を継承し、コピー部品の保持レジスタも維持する。
テストファイルとline.txtは一時ディレクトリ内だけで扱い、作業ディレクトリの既存ファイルを上書きしない。


## 実装・検証結果（2026-11-20）

ロード部品14ケースを先に確認してからVMへ統合した。統合後もロード14・VM経由20ケースが通過。
新規入出力265ケース（shell 49、editor直接94、shell経由94、起動・復帰28）が通過した。
shellは139命令556 byte + 14文字列271 byte = 827 byte（末尾の次0x33B）。
editorは157命令628 byte + 15文字列337 byte = 965 byte（末尾の次0x3C5）。ともに0x500以下。
shell/editor/run-childの再アセンブル一致、line.txt本文一致、旧サンプルのbyte保持を確認した。
Day 44の回帰セット全体、VM自己テスト、既存cmp、別ビルドVMと通常の起動入口も確認した。
コードテスト・生成物の実測と成功・失敗の生ログはwp/codex-day45-evidence.json、
開始状態はwp/codex-day45-start.json、詳細は[解説](../test-code-explanations/045-shell-editor.md)を参照。
最小仕様の未決事項なし。記事・公開・コミットは行っていない。

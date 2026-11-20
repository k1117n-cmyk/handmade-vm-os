# Day 45 shell / editorの分離

## 使い方と責務

リポジトリのルートで`make run-shell`を実行する。
Command>でeditを入力すると、独立したeditorのEditor>へ移る。
set hello → show → save → quitでshellへ戻り、もう一度edit → showを実行すると空内容を表示する。
load → showでline.txtのhelloを復元できる。editorのquitの後、shellのquitで終了する。
実際の保存は作業ディレクトリのline.txtを直接上書きする。保存途中の失敗では旧ファイル保持を保証しない。
quitとEOFでは自動保存・確認質問をしない。

shellはhelp / echo / run / edit / quitを持ち、編集領域を操作しない。
editorはhelp / set / show / save / load / quitを持ち、別プログラムを起動しない。
それぞれのhelpと責務外コマンドの拒否を固定期待値で照合する。
editor単独なら`make run-editor`。単独起動時のquitはそのVMを終了する。

## ロード部品からVM統合へ

notes/program-launch.hのload_fixed_programがread・容量・末尾確認・closeを共通処理する。
load_child_programは既存run-child.bin、load_editor_programはeditor.binというhost内固定文字列を渡す。
VMからパスを受け取る機能はない。既存のSYSCALL 6と未実装の7を維持し、editor用に8を追加した。

まずnotes/045-shell-editor-test.cをロード部品だけでビルド・実行し、14ケースを確認した。
editorの欠落・0〜3 byte・4 byte・1 MiB前後・open/read/closeと末尾確認の失敗に加え、
旧load_child_programがrun-child.binを選ぶことを検査する。

その後、notes/vm.cへ統合した。launch_loaded_program(loader)が子VMの確保・初期化・is_child設定・ロード・実行・解放を行い、
launch_programとlaunch_editorが固定のロード関数を選ぶ。SYSCALL 8は結果だけを親R0に返す。
正常HALTなら0、確保・ロード失敗やHALT以外の停止なら1。子の結果の後、親はsyscallの次の命令から進む。
親の全メモリ、R1〜R7、SP、zero flag、runningは保持される。
子VMから6/8を呼ぶ場合はopen・子確保を行わずR0=1で子へ戻り、多重起動しない。

```sh
make test-shell-editor-parts
```

同じテストを-DLAUNCH_VM_TESTでビルドすると、ロード14に加えてVM経由20ケースを検査する。
fetch/decode/executeを通し、親の全メモリ・全レジスタ・SP・フラグ・running・is_child・復帰PCを比較する。
HALT、不正命令、RETのunderflow、子によるメモリ・レジスタ・スタック・フラグ変更、連続起動、
子からの6/8、旧6の起動先、確保・open/read/close・容量・末尾確認の失敗、入力EOFを含む。
read・close等でロードが失敗した子は実行せず、HALT表示が出ないことも確認する。
stdioとmalloc/freeの置換で故障を作り、開いたファイル・確保した子VMの解放数も照合する。

## 入出力265ケースと生成物

```sh
make test-shell-editor
```

tools/test-shell-editor.pyは、VMの実際のstdout全文・空stderr・終了コード0とファイルbyteを固定期待値で検査する。
49ケースがshell入力、94ケースがeditor直接起動、同じ94ケースがshell経由、28ケースが起動・復帰などの統合検査。
set単独・区切りだけ・短縮・空内容、コマンドの責務分離、非空引数拒否、空白、タブ、CRLF、入力上限、
NUL、UTF-8、EOF、save/loadの成功・失敗・保持、空と63 byte、不正な保存ファイルを含む。
line.txtと差し替えた子プログラムは全て一時ディレクトリ内に置き、操作後のファイル保持も照合する。

統合検査では明示save→editor再起動→空内容→loadによる復元、未保存内容の破棄、
run/editの固定起動先、欠落・短いbin・容量境界・実行エラー後のshell継続、共有入力とEOF、
子の6/8拒否、1000回のeditorセッションと1000回の子VM初期化、monitor復帰とshell再起動を検査する。
初期化確認用の小さな子はレジスタ・メモリ・zero flagを検査し、メモリ・フラグ・スタックを変更してHALTする。
スタック状態は対応済みのCALLIで作る。最初のテスト作成時に未対応PUSHを使った失敗も生ログへ残している。
失敗した実行[5]と成功した実行[6]の間に、tools/test-shell-editor.pyへout of range fetchのケースを1件追加した。この追加によりlaunchは27件から28件、合計は264件から265件となった。

| 生成物 | 命令 | 命令byte | 文字列 | 文字列byte | 合計byte | 末尾の次 |
|---|---:|---:|---:|---:|---:|---|
| shell.bin | 139 | 556 | 14 | 271 | 827 | 0x33B |
| editor.bin | 157 | 628 | 15 | 337 | 965 | 0x3C5 |

双方とも0x500より手前。入力0x500〜0x53Fとeditorの編集0x600〜0x63F、下向きスタックと重ならない。
copy_line / split_line / read_line / compare_commandは既存asmと同じ規約・実体を継承する。
テストでは文字列領域の全byte、syscallの必要な番号と責務外の番号不在を検査し、
shell/editor/run-childそれぞれを再アセンブルしたbinとの全byte一致を照合する。
旧program-launcherは1198 byte、run-childは3命令12 byteを維持した。
既存のasm・bin・旧テスト・small-asm・writer・line-storage.hの開始時ハッシュも保持した。

Day 44回帰セット全体とVM自己テスト、既存binのcmpを確認済み。
別ビルドVMでmonitor・hello・shellからのeditor起動を確認し、Makefileのrun-shell/run-editorの通常入口も確認した。
[仕様カード](../specs/045-shell-editor.md)、wp/codex-day45-start.jsonの開始状態、
wp/codex-day45-evidence.jsonの成功・失敗を含む生ログ、wp/codex-completion-report.mdを参照。

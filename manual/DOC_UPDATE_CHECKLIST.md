# Documentation Update Checklist

このチェックリストは、実装や文書更新の最後に、関連ファイルの更新漏れを確認するために使う。

Codexは、コード変更、サンプル追加、ツール追加、記事更新を行ったターンの最後に、このファイルを確認する。

## 最初に確認すること

```sh
git status --short
git diff --stat
git diff --name-only
```

確認すること:

```text
意図しないファイルが変更されていないか
新規ファイルがgit管理対象に入るべきものか
articles/配下のファイルはgit管理対象外か
```

## 命令を追加したとき

必ず確認するファイル:

```text
manual/specs/NNN-name.md
notes/NNN-name-test.c
manual/test-code-explanations/NNN-name-test.md
manual/instruction-types.md
manual/README.md
README.md
notes/vm.c
manual/HANDOFF.md
```

必要に応じて確認するファイル:

```text
HANDWRITING_GUIDE.md
manual/NEXT_INSTRUCTION_GUIDELINES.md
manual/reference/instruction-fields.md
programs/README.md
programs/*.bin
tools/*
```

確認すること:

```text
命令番号 type/op が一覧と一致しているか
仕様、テスト、解説で命令名と番号が一致しているか
notes/vm.c に統合した場合、個別テスト後に統合VMでも確認したか
READMEの試し方に必要なコマンドがあるか
manual/HANDOFF.md に完了内容と次回候補を書いたか
```

## VM機能を追加したとき

例:

```text
外部バイナリローダー
trace
debug dump
実行オプション
```

必ず確認するファイル:

```text
manual/specs/NNN-name.md
notes/NNN-name-test.c
manual/test-code-explanations/NNN-name-test.md
notes/vm.c
README.md
manual/README.md
manual/HANDOFF.md
```

必要に応じて確認するファイル:

```text
manual/instruction-types.md
HANDWRITING_GUIDE.md
ROADMAP.md
programs/README.md
tools/*
```

確認すること:

```text
その機能が命令なのかVM起動機能なのかを分けて書いたか
引数や使い方をREADMEに書いたか
失敗時の動作を仕様に書いたか
既存の引数なし実行が壊れていないか
```

## programs/*.bin を追加または変更したとき

必ず確認するファイル:

```text
programs/README.md
README.md
manual/HANDOFF.md
```

必要に応じて確認するファイル:

```text
tools/*
manual/specs/*
manual/test-code-explanations/*
```

確認すること:

```text
xxd programs/name.bin で中身を確認したか
cc notes/vm.c -o /tmp/handmade-vm を実行したか
/tmp/handmade-vm programs/name.bin で期待出力を確認したか
その .bin を再生成する方法があるか
```

## tools/* を追加または変更したとき

必ず確認するファイル:

```text
tools/*
README.md
manual/README.md
manual/HANDOFF.md
```

必要に応じて確認するファイル:

```text
programs/README.md
manual/specs/*
manual/test-code-explanations/*
```

確認すること:

```text
ツールの入力と出力がREADMEから分かるか
生成物をVMで実行して確認したか
一時出力は/tmp、残すサンプルはprograms/に置いているか
```

## 記事ファイルを変更したとき

対象:

```text
articles/*
```

確認すること:

```text
公開済み記事なら、ユーザーが指定した箇所以外を変更していないか
articles/はgit管理対象外なので、git diffに出ない前提で直接内容を確認したか
記事内の次回予告がROADMAP.mdと矛盾していないか
```

## 終了前の標準確認

コードやツールを変更した場合:

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm
/tmp/handmade-vm programs/hello.bin
```

`tools/write-hello-bin.c` に関係する場合:

```sh
cc tools/write-hello-bin.c -o /tmp/write-hello-bin
/tmp/write-hello-bin
xxd programs/hello.bin
/tmp/handmade-vm programs/hello.bin
```

個別テストをまとめて確認する場合:

```sh
for f in notes/*-test.c; do out="/tmp/$(basename "$f" .c)"; cc "$f" -o "$out" || exit 1; "$out" >/tmp/handmade-vm-test-output || exit 1; done
```

## コミット前チェック

```sh
git status --short
git diff --stat
git diff --cached --stat
```

確認すること:

```text
manual/HANDOFF.md が最新の再開点を示しているか
README.md にユーザーが試すための最短コマンドがあるか
ROADMAP.md と NEXT_INSTRUCTION_GUIDELINES.md が次の方針と矛盾していないか
不要な一時ファイルや実行ファイルがgitに入っていないか
```

## 今回の企画で守ること

```text
既存教材は、組み合わせて使う部品ではなく参考資料
まず部品テスト
問題がなければ統合VMへ実装
最後にVMからprograms/*.binを起動して確認
作業終わりにmanual/HANDOFF.mdを更新
```

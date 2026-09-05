# Publishing Checklist

GitHubで公開する前に確認すること。

## ローカル確認

```sh
git status --short
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm
```

期待する出力:

```text
A
CPU halted.
```

警告も確認する場合:

```sh
cc -Wall -Wextra -pedantic notes/vm.c -o /tmp/handmade-vm-warn
```

## 公開対象

GitHubに入れる主なファイル:

- `README.md`
- `ROADMAP.md`
- `HANDWRITING_GUIDE.md`
- `vm-builder-eye-practice.md`
- `instruction-fields.md`
- `HALT_FOR_BEGINNERS.md`
- `WORKBOOK_001_HALT.md`
- `notes/`
- `LICENSE`
- `ATTRIBUTION.md`

記事原稿の作業ファイルは `articles/` に置き、GitHub公開対象から外します。

## GitHub repository

まだremoteがない場合は、GitHubで空のリポジトリを作ってから設定します。

```sh
git remote add origin git@github.com:k1117n-cmyk/handmade-vm-os.git
git push -u origin main
```

HTTPSを使う場合:

```sh
git remote add origin https://github.com/k1117n-cmyk/handmade-vm-os.git
git push -u origin main
```

## Repository description

GitHubのDescription例:

```text
自作CPU、自作VM、自作OSへ進むための手書き学習ノート
```

Websiteには公開記事を設定します。

```text
https://pc-fan.net/handmade-vm-builder-eye/
```

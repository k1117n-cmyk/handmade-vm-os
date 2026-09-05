# Handmade VM OS

自作CPU、自作VM、自作OS、自作シェル、自作エディターへ進むためのシリーズ用リポジトリです。

完成コードを急いで作るのではなく、1つの命令を小さく分解し、仕様カード、手計算、`fetch`、`decode`、`execute`、Cコードへの置き換えを順番に確認します。

公開記事:

- [自作OSへの第一歩: 写経からVMを作る人の目線へ](https://pc-fan.net/handmade-vm-builder-eye/)

## まず読むもの

最初は次の順番で読むと、目的と練習方法が追いやすいです。

1. [ROADMAP.md](ROADMAP.md)
2. [HANDWRITING_GUIDE.md](HANDWRITING_GUIDE.md)
3. [vm-builder-eye-practice.md](vm-builder-eye-practice.md)
4. [instruction-fields.md](instruction-fields.md)
5. [WORKBOOK_001_HALT.md](WORKBOOK_001_HALT.md)

`HALT` をかなりやさしく分解した説明は [HALT_FOR_BEGINNERS.md](HALT_FOR_BEGINNERS.md) にあります。

## このリポジトリで扱うこと

- 1MB の byte-addressed memory を持つ小さなVM
- 32bit big-endian instruction fetch
- `R0`-`R7` と `PC` を使う最小CPU状態
- `HALT`, `MOVI`, `SYSCALL`, `LDB` などの命令を1つずつ実装する練習
- アセンブリ表記、命令field、Cコードの対応づけ

最終的には次の形を目指します。

```text
host macOS/Linux
  -> 自作VM本体 C
  -> 自作CPU ISA
  -> 自作OS
  -> 自作shell
  -> 自作editor
```

## ノート構成

```text
notes/
  000-machine-state.md
  001-halt.md
  002-fetch.md
  003-movi.md
  004-syscall.md
  005-decode.md
  006-syscall-print-char.md
  007-syscall-print-string.md
  008-ldb.md
  vm.c
```

各 `*-test.c` は、その日の命令やVM部品を小さく確認するためのテストコードです。

## 試し方

Day 8 時点の最小VMは `notes/vm.c` です。

```sh
cc notes/vm.c -o /tmp/handmade-vm
/tmp/handmade-vm
```

期待する出力:

```text
A
CPU halted.
```

個別の練習コードも同じようにコンパイルして実行できます。

```sh
cc notes/008-ldb-test.c -o /tmp/008-ldb-test
/tmp/008-ldb-test
```

## 学習の型

新しい命令を見たら、次の3つの目線で分けます。

```text
1. アセンブリを書く人の目線
2. 命令をdecodeするVMの目線
3. 実際に値を動かすCコードの目線
```

例:

```asm
LDB R0, [R1]
```

```text
Assembly        C
R0              regs[0]
R1              regs[1]
[R1]            memory[regs[1]]
LDB R0, [R1]    regs[0] = memory[regs[1]]
```

この置き換えを手で追う練習用の短いガイドが [vm-builder-eye-practice.md](vm-builder-eye-practice.md) です。

## ライセンスと出典

コードと文書は [MIT License](LICENSE) で公開します。

参考にしている教材や記事は [ATTRIBUTION.md](ATTRIBUTION.md) にまとめています。

GitHub公開前の確認手順は [PUBLISHING.md](PUBLISHING.md) にまとめています。

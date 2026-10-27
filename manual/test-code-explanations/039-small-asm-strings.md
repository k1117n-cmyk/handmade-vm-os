# Day 39: 命令と文字列を同じソースへ置く

対象は `tools/small-asm.c` と `tools/test-small-asm-strings.py` です。VM命令の追加ではなく、ホスト側の変換ツールの拡張なので、notesの個別CテストやVM本体の変更はありません。

## まず表示する

```sh
make run-asm-string
```

```text
Hello, VM!
CPU halted.
```

`programs/asm-string.asm` は4命令と文字列を含み、28 byteを生成します。最初のJUMPで文字列を飛び越し、MOVIで文字列の先頭をR0へ入れ、SYSCALL 1で終端0まで表示します。

## 位置を手で数える

| 位置 | 内容 | byte数 |
|---|---|---|
| 0x00 | JUMP start | 4 |
| 0x04 | Hello, VM! + LF + 終端0 | 12 |
| 0x10 | MOVI R0, message | 4 |
| 0x14 | SYSCALL 1 | 4 |
| 0x18 | HALT | 4 |

`.align 4` は命令ではなく、出力位置を4の倍数へ揃える指示です。この例は既に揃っているため0 byte。本文を `Hi!\n` にすると5 byte（終端込み）になり、0x09〜0x0Bを0で埋め、startは0x0Cになります。ラベル自体は出力しません。

## 実装の対応

- `normalize_line`: 引用符の内外を追い、外のコメントを除去して大文字へ揃える。引用符内の `; # : ,` や大小文字は保存する。バックスラッシュの次の文字は引用符の開閉として扱わない。
- `prepare_line`: 文字列内のコロンをラベルと誤認せず、現在のbyte位置を名前へ登録する。
- `assemble_directive`: `.STRING` をASCIIと5種類のエスケープからbyte列へ変換し、最後に0を加える。`.ALIGN` は `(4 - address % 4) % 4` 個の0を生成する。
- `parse_target`: 従来の分岐用の数値・名前解決をMOVIでも使う。第1パスは仮の0、第2パスで実アドレスへ置き換える。
- `main`: 各パスでaddressを0に戻す。指示なら生成byte数、命令なら4を加える。instruction_countは成功メッセージの命令数専用とする。

命令はこれまで通りbig-endianです。`MOVI R0, message` はmessage=4なので `0x40000004`、ファイルでは `40 00 00 04`。文字列は命令にまとめず、Hなら0x48、eなら0x65を1 byteずつ並べます。

第1パスと第2パスが同じ文字列解析・整列計算を使うことが大切です。エスケープをソース上の2文字として数えると、実出力とラベル位置がずれます。

## テストの読み方

```sh
make test-small-asm test-small-asm-labels test-small-asm-strings
xxd programs/asm-string.bin
```

文字列用52ケースは、固定byte列との一致、0〜3 byteの詰め物、前方・後方MOVI参照、0xFFFFFのデータ参照、1 MiB境界、各種不正入力、出力ファイル保持、VM実行を自動照合します。期待byte列はアセンブラを使わずPython側で組み立てます。

Day 38の44ケースは維持し、MOVIのラベルを拒否する1ケースを受理するケースへ変更しました。既存57ケースもそのまま実行します。

`.align 4` は実行を移動させません。JUMPやHALTで文字列を命令として読まない流れを記述してください。ラベルにはコード用・データ用の型を付けていないため、分岐先の実行可能性までは検査しません。

## 次の単位

echo全体の移行は次回です。専用writerの固定配置と入力バッファ0x500を確認し、現行構文で安全に表せるか、絶対配置指示が必要かを仕様カードで決めます。文字列表示の最小例が通ったことと、echoを移行済みであることは区別します。

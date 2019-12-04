# bptree-nvm

NVMを使うB<sup>+</sup>-Treeの実験用コード諸々

## 実行方法

cloneしたリポジトリで`make`とするとbuildディレクトリができてその中に実行ファイルができる

`make type=xxx`とすることで作る種類が変わる

- simple = シングルスレッドの一番シンプルなもの
- concurrent = マルチスレッドで動作するもの．葉ノードではロックを使う
- nvhtm = マルチスレッドで動作するもの．葉ノードでは[NV-HTM](https://bitbucket.org/daniel_castro1993/nvhtm/src/master/)を使う

main関数はsrc/test内のテスト用コード，src/benchmark内のベンチマーク用コードにある

res/以下に実験の結果が置いてある

## 未mergeブランチ一覧

- merged-bitmap：bitmapを廃止してfingerprintの0を特別扱いすることで葉ノードを圧縮する実装
- double-buffering：ログ領域を二つ用意して切り替えるようにすることで停止時間を減らす実装
- plain-nvhtm-tree：通常のB<sup>+</sup>-TreeをNVHTMを使ってpersistent化した場合の実装

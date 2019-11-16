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

## ブランチ一覧

- re-implement-updateParent：updateParent（insertParentに改名）の仕様変更に伴い実装し直す用（merge済み）
- implement-delete：deleteを実装する用（merge済み）
- test-single-thread：シングルスレッドでの動作確認用コードを追加する用（merge済み）
- concurrent：HTMを使って並行化する用（merge済み）
- allocator：並行アロケータの実装用（merge済み）
- benchmark：ベンチマークプログラム作成用（merge済み）
	- ベンチマーク自体は動作する
	- グラフ生成がイマイチ
- implement-replaced-minnvm：NV-HTMのNVMエミュレータを実NVMに置き換える用（merge済み）
	- 結局エミュレータ自体は空にしただけ
	- NH_allocのmmapへの置き換え
	- shmgetによる共有メモリで実装されていたログをmmapに置き換え

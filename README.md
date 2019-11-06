# bptree-nvm

## 実行方法

cloneしたリポジトリで`./compile.sh -u`とするとbuildディレクトリができてその中に実行ファイルができる

main関数はsrc/test内のテスト用コードにある

## ブランチ一覧

- re-implement-updateParent：updateParent（insertParentに改名）の仕様変更に伴い実装し直す用（merge済み）
- implement-delete：deleteを実装する用（merge済み）
- test-single-thread：シングルスレッドでの動作確認用コードを追加する用（merge済み）
- concurrent：HTMを使って並行化する用（作業中）

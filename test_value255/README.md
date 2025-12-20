# Value255 単体テスト

このディレクトリは、Value255クラスの単体テストを実行するためのESP-IDFプロジェクトです。

## テスト内容

以下の機能をテストします：

1. 空のValue255の作成
2. 小さいデータ（インラインストレージ）の作成と取得
3. 大きいデータ（ヒープストレージ）の作成と取得
4. 最大サイズ（255バイト）のValue255の作成
5. 無効なパラメータでの作成失敗
6. クローン機能
7. 等価比較演算子（`==`）
8. 比較演算子テスト
9. ムーブ代入演算子
10. 文字列変換（16進数表示）

## ビルドと実行方法

### 1. テストディレクトリに移動

```bash
cd /home/riru/trial-cpp/test_value255
```

### 2. ターゲットの設定（初回のみまたはターゲット変更時）

```bash
idf.py set-target esp32c6
```

または、お使いのターゲットに合わせて変更してください（esp32, esp32s3など）。

### 3. ビルド

```bash
idf.py build
```

### 4. デバイスへのフラッシュと実行

ESP32デバイスをUSBで接続してから：

```powershell
usbipd attach --wsl --busid 1-1
```

```bash
idf.py flash monitor
```

または、個別に実行：

```bash
idf.py flash      # デバイスにフラッシュ
idf.py monitor    # シリアルモニタで出力を確認
```

### 5. テスト結果の確認

シリアルモニタに以下のような出力が表示されます：

```
...
Unity test run 1 of 1
test_create_empty_value:PASS
test_create_inline_value:PASS
test_create_heap_value:PASS
test_create_max_size_value:PASS
test_create_invalid_parameters:PASS
test_clone_value:PASS
test_equality_operator:PASS
test_comparison_operator:PASS
test_move_assignment:PASS
test_str_conversion:PASS

-----------------------
10 Tests 0 Failures 0 Ignored 
OK
```

すべてのテストがPASSすれば成功です。

## モニタの終了

`Ctrl + ]` でシリアルモニタを終了します。

## その他のコマンド

### クリーンビルド

ビルド成果物を削除して最初からビルドし直す場合：

```bash
idf.py fullclean
idf.py build
```

### ポートの指定

デバイスが自動検出されない場合、ポートを明示的に指定できます：

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

ポート名はシステムによって異なります（Linux: `/dev/ttyUSB*` or `/dev/ttyACM*`、Windows: `COM*`）。

### ボーレートの変更

デフォルトのボーレートが合わない場合：

```bash
idf.py -b 115200 flash monitor
```

## プロジェクト構造

```
test_value255/
├── CMakeLists.txt          # プロジェクト設定
├── README.md               # このファイル
├── main/
│   ├── CMakeLists.txt      # メインコンポーネント設定
│   └── test_main.cpp       # テストコード本体
└── sdkconfig               # ESP-IDF設定（自動生成）
```

## テストの追加方法

新しいテストケースを追加するには、[main/test_main.cpp](main/test_main.cpp)を編集してください：

1. 新しいテスト関数を作成（例: `void test_new_feature(void)`）
2. `app_main()`関数内に`RUN_TEST(test_new_feature);`を追加
3. プロジェクトを再ビルドして実行

## 注意事項

- このテストプロジェクトは、親プロジェクトの`components/value`と`components/util`を参照しています
- テストはESP-IDFのUnityフレームワークを使用しています
- 実機（ESP32デバイス）が必要です

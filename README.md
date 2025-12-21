| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Trial C++ Project

ESP-IDF C++23 project with component-based unit tests.

## Build and Flash

### Normal Application Build
```bash
cd trial-cpp
idf.py build
idf.py flash monitor
```

### Unit Test Build and Run
```bash
# ユニットテストを有効にしてビルド
cd trial-cpp/test
idf.py build

# デバイスに書き込んで実行
idf.py flash monitor

# デバイス起動後、Enterキーを押すとテストメニューが表示されます
# テスト番号を入力するか、"*"で全テスト実行
```

テスト後、通常ビルドに戻す：
```bash
idf.py build
```

## Project Structure

```
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── hello_world_main.cpp       # メインアプリケーション（通常モード/テストモード切替）
│   └── Kconfig.projbuild          # テストモード設定
├── components/
│   ├── value/
│   │   ├── CMakeLists.txt
│   │   ├── value255.cpp
│   │   └── include/value255.hpp
│   ├── value_test/                # valueのユニットテスト（独立コンポーネント）
│   │   ├── CMakeLists.txt
│   │   └── test_value255.cpp
│   ├── util/
│   └── machine/
└── README.md
```

## Adding New Tests

新しいコンポーネントのテストを追加する場合：

1. `components/<component_name>_test/` ディレクトリを作成
2. テストファイル `test_*.cpp` を作成
3. `<component_name>_test/CMakeLists.txt` を作成：
```cmake
idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity <component_name> <dependencies>)
```
4. `main/CMakeLists.txt`の`PRIV_REQUIRES`に`<component_name>_test`を追加

テストは自動的にunityフレームワークに登録されます。
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.

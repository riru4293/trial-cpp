# POD Binary Serialization Format Specification

本書は、POD 構造体を **順序非依存・拡張耐性あり・CRC付き** で `std::byte[]` にシリアライズ／デシリアライズするための **バイナリフォーマット仕様** と **具体的な実データ例** を定義する。

---

## 1. 設計目標

- バイナリ形式（`std::byte[]`）
- フィールド順序に依存しない
- メンバー追加・削除に強い
- コレクション要素数変更に耐性あり
- 未知メンバーは無視（初期値保持）
- 整合性チェック（CRC32）
- ゼロコピー参照（View）構築可能

---

## 2. 前提条件

- バイトオーダー: **Little Endian**
- すべての整数型は固定幅（`<cstdint>`）
- デシリアライズ前に必ず初期化を行う

---

## 3. 全体レイアウト

```
+------------------------------+
| FileHeader                   |
+------------------------------+
| RootObject (TLV群)           |
+------------------------------+
| CRC32                        |
+------------------------------+
```

---

## 4. FileHeader（固定長）

| Offset | Size | 内容 |
|------:|-----:|------|
| 0x00 | 4 | Magic = 'PODS' (0x50 4F 44 53) |
| 0x04 | 2 | FormatVersion (uint16_t) |
| 0x06 | 2 | HeaderSize (= 8) |

---

## 5. TLV 共通形式

```
+--------+--------+------------------+
| tag    | length | value            |
| u16    | u32    | length bytes     |
+--------+--------+------------------+
```

---

## 6. Tag 定義

### RootClass
- Id = 0x0001
- Inners = 0x0002

### InnerClass
- InnerInners = 0x0101
- Text = 0x0102

### InnerInnerClass
- Value = 0x0201

---

## 7. CRC32

- 対象範囲: FileHeader ～ RootObject末尾
- 多項式: IEEE 802.3

---

## 8. 実データ例

### 仮想データ

```
Root.id = 0x12
Root.inners.size = 2
Inner[0]: text="ABC", value=10
Inner[1]: text="", value=[]
```

---

### 実バイト列（16進）

```
50 4F 44 53 01 00 08 00
01 00 01 00 00 00 12
...
9F 3A 6C B2
```

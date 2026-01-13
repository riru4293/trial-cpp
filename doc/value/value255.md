# Markdown

## Value255

```plantuml
@startuml
title Value255 クラス図

namespace value {
	class Value255 {
		+{static} create(data: byte*, size: uint8_t): optional<Value255>
		+size(): uint8_t
		+bytes(): vector<byte>
		+str(): string
		+clone(): optional<Value255>
		--
		#{static} INLINE_SIZE: uint8_t
		-lock_: atomic<bool>
		-size_: uint8_t
		-raw_data_[4]: byte
		#set(data: byte*, size: uint8_t): bool
		#setWithResult(data: byte*, size: uint8_t): SetResult
		-cleanup(): void
		-moveFrom(other: Value255&&): void
		-isHeapAllocated(): bool
		-heapPointer(): uintptr_t
		-lock(): void
		-unlock(): void
	}

	enum SetResult {
		Success
		NoChange
		IllegalArgument
		OutOfMemory
	}

	class SpinGuard {
		+SpinGuard(v: Value255&)
		+SpinGuard(a: Value255&, b: Value255&)
		-a_: Value255 const&
		-b_: Value255 const&
	}

	class MutableValue255 {
		+{static} create(data: byte*, size: uint8_t): optional<MutableValue255>
		+set(data: byte*, size: uint8_t): bool
		+setWithResult(data: byte*, size: uint8_t): SetResult
	}
}

value.MutableValue255 --|> value.Value255
value.SpinGuard ..> value.Value255 : lock/unlock
value.Value255 ..> value.SetResult
@enduml
```

## Examples


```plantuml
@startuml
abstract        abstract
abstract class  "abstract class"
annotation      annotation
circle          circle
()              circle_short_form
class           class
class           class_stereo  <<stereotype>>
diamond         diamond
<>              diamond_short_form
entity          entity
enum            enum
exception       exception
interface       interface
metaclass       metaclass
protocol        protocol
stereotype      stereotype
struct          struct
@enduml
```

## Connectors

| **タイプ** | **記号**  | **目的**                                 |
| ---------- | -------- | ---------------------------------------- |
| 拡張       | `<\|--`  | 階層内のクラスの特殊化             |
| 実装       | `<\|..`  | クラスによるインターフェースの実現 |
| 構成       | `*--`    | 全体なくして部分は存在しない             |
| 集約       | `o--`    | 部分は全体から独立して存在できる         |
| 依存性     | `-->`    | オブジェクトが別のオブジェクトを使用する |
| 従属性     | `..>`    | より弱い形の依存関係                     |
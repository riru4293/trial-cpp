# Markdown

## machine

```plantuml
@startuml
title Machine クラス図

namespace machine {
    class Property {
        +code(): uint8_t
        +spec(): Spec
        +value(): Value
        +setValue(data: byte*, size: uint8_t): SetResult
        +str(): string
        --
        #{static} SetResult enum
        -code_: uint8_t
        -spec_: property::Spec
        -value_: property::MutableValue
        -isWritable(): bool
        -isValidValue(): bool
        -updateValue(): SetResult
    }

    enum SetResult {
        Success
        NoChange
        IllegalArgument
        Forbidden
        InternalError
    }

    namespace property {
        class Spec {
            +permission(): Permission
            +resolution(): Resolution
            +initialValue(): Value255
            +minimumValue(): Value255
            +maximumValue(): Value255
            +isWithinRange(data: byte*, size: uint8_t): bool
            +str(): string
            --
            -format_: Format
            -permission_: Permission
            -resolution_: Resolution
            -initial_: Value255
            -minimum_: Value255
            -maximum_: Value255
        }

        class Format {
            +kind(): Kind
            --
            -kind_: Kind
        }

        class Permission {
            +kind(): Kind
            --
            -kind_: Kind
        }

        class Resolution {
            +kind(): Kind
            --
            -kind_: Kind
        }
    }
}

machine.Property *-- machine.property.Spec : owns
machine.Property *-- machine.property.Value255 : owns value
machine.property.Spec ..> machine.property.Format
machine.property.Spec ..> machine.property.Permission
machine.property.Spec ..> machine.property.Resolution
machine.property.Spec *-- machine.property.Value255 : initial, min, max
machine.Property ..> machine.SetResult
@enduml
```

解説

```text
以下の要素を含めています：

Property: 主要クラス、code/spec/valueを保持
SetResult: プロパティ値設定時の戻り値
Spec: プロパティ仕様（Format, Permission, Resolution, Value255）
Format, Permission, Resolution: Spec構成要素
関連は以下のように表現しています：

*--: 構成（PropertyはSpecとMutableValueを完全に所有）
o--: 集約（SpecはFormat/Permission/Resolutionを参照）
..>: 弱依存（PropertyはSetResultを戻り値で返す。でもメンバーではなく一時的）
```

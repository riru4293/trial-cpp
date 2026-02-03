#pragma once

/**
 * @namespace machine
 * @brief Provides foundational functionality for communication with external machine.
 *     @n ja: （外部機械との通信に関する基礎的な機能を提供する名前空間） */
/**
 * @details
 * This namespace defines abstractions and implementations used to identify,
 * access, and operate on external machine through a hierarchical addressing model.
 * @n
 * Only properties are operational targets. Properties are accessed through
 * a hierarchical address structure (unit, component), but intermediate levels
 * have no behavior and exist solely for addressing purposes.
 * @n
 * The logical structure can be illustrated as follows:
 * - machine
 *   - unit[]
 *     - component[]
 *       - property[]
 *
 * Classes that directly participate in communication, such as value
 * representations, address definitions, and transport mechanisms,
 * are expected to reside directly under this namespace.
 * @n @n ja: @n
 * この名前空間は、階層的なアドレス指定モデルを通じて外部マシンを識別、アクセス、
 * および操作するために用いられる抽象化と実装を定義します。
 * @n
 * 操作対象はプロパティのみです。プロパティは階層的なアドレス構造（ユニット、コンポーネント）を
 * 通じてアクセスされますが、中間レベルには動作はなく、アドレス指定のためだけに存在します。
 * 論理構造に関しては、英文説明側に記載されている通りですので、そちらを参照してください。
 * @n
 * 値の表現、アドレス定義、伝送メカニズムなど、通信に直接参加するクラスは、
 * この名前空間の直下に配置されることが期待されます。
 */
namespace machine
{
    /**
     * @namespace machine::property
     * @brief Groups concepts and operations related to external machine properties.
     *     @n （ja: 外部機械のプロパティに関連する概念と操作をまとめた名前空間） */
    namespace property
    {
    }
}

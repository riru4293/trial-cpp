#pragma once

/* C++ Standard Library */
#include <utility>

namespace util
{
    /** @brief RAII guard with user-provided lock/unlock callables. */
    /**
     * @details
     * This template provides automatic acquisition and release of locks using
     * user-provided callable objects for locking and unlocking.
     * It supports both single-instance and dual-instance locking.
     * Deadlock prevention when locking the same instance is controlled via the
     * `same` flag: when `same=true`, only one lock is acquired; when `same=false`,
     * both locks are acquired in order (a, then b).
     *
     * @tparam LockA   The callable type for acquiring the first lock.
     * @tparam UnlockA The callable type for releasing the first lock.
     * @tparam LockB   The callable type for acquiring the second lock.
     * @tparam UnlockB The callable type for releasing the second lock.
     *
     * @par Example Usage
     * @code
     * // Single instance lock
     * {
     *     auto lock = []() noexcept { myObj.lock(); };
     *     auto unlock = []() noexcept { myObj.unlock(); };
     *     util::SpinGuard guard( lock, unlock, lock, unlock, true );
     *     // Critical section - myObj is locked
     * } // myObj is automatically unlocked
     *
     * // Dual instance lock
     * {
     *     auto lockA = []() noexcept { objA.lock(); };
     *     auto unlockA = []() noexcept { objA.unlock(); };
     *     auto lockB = []() noexcept { objB.lock(); };
     *     auto unlockB = []() noexcept { objB.unlock(); };
     *     util::SpinGuard guard( lockA, unlockA, lockB, unlockB, false );
     *     // Both objA and objB are locked
     * } // Unlocked in reverse order
     * @endcode
     *
     * @attention
     * - This guard is **not reentrant**. Attempting to lock the same instance
     *   multiple times will result in deadlock.
     * - The guard is not movable or copyable to prevent accidental lock release.
     * - Locks are held for the entire lifetime of the guard object.
     * - Lock/Unlock callables should be `noexcept`; exceptions during lock or unlock
     *   can leave resources in an inconsistent state.
     * - Unlock callables are stored and invoked in the destructor. They must remain
     *   valid for the entire lifetime of the guard (do not capture dangling references).
     * - Callables should be invocable with a signature compatible with `void() noexcept`.
     * - Unlocking occurs in reverse order (`b` then `a`). Ensure this order respects
     *   resource dependencies.
     */
    template<typename LockA,         typename UnlockA,
             typename LockB = LockA, typename UnlockB = UnlockA>
    class SpinGuard
    {
    public:
        /** @brief Constructs a guard for single or dual instances. */
        /**
         * @details
         * If `same` is true, only the first lock is acquired (for single instance).
         * Otherwise, both locks are acquired in order (a, then b) and released in reverse order.
         *
         * @attention
         * If `same` is false and both `lock_a` and `lock_b` operate on the
         * same instance/resource, a deadlock will occur. When locking the
         * same instance with both callables, set `same=true` to acquire only
         * one lock and avoid deadlock.
         *
         * @param lock_a   [in] Callable to acquire the first lock.
         * @param unlock_a [in] Callable to release the first lock.
         * @param lock_b   [in] Callable to acquire the second lock.
         * @param unlock_b [in] Callable to release the second lock.
         * @param same     [in] Must be `true` if `a` and `b` are locking
         *                      the same instance, and `false` otherwise.
         */
        explicit SpinGuard( LockA lock_a, UnlockA unlock_a,
                            LockB lock_b, UnlockB unlock_b,
                            bool same
        ) noexcept
            : same_( same )
            , unlock_a_( std::move( unlock_a ) )
            , unlock_b_( std::move( unlock_b ) )
        {
            if ( same_ ) { lock_a(); }
            else         { lock_a(); lock_b(); }
        }

        SpinGuard( SpinGuard const & ) = delete;
        SpinGuard &operator=( SpinGuard const & ) = delete;
        SpinGuard( SpinGuard && ) = delete;
        SpinGuard &operator=( SpinGuard && ) = delete;

        /** @brief Destructor - unlocks in reverse order. */
        ~SpinGuard()
        {
            if ( same_ ) { unlock_a_(); }
            else         { unlock_b_(); unlock_a_(); }
        }

    private:
        bool const same_;
        UnlockA const unlock_a_;
        UnlockB const unlock_b_;
    };

} // namespace util

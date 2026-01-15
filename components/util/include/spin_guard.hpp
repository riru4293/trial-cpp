#pragma once

namespace util
{
    /** @brief RAII-based spinlock guard for types with lock()/unlock() methods. */
    /**
    * @details
    * This template provides automatic acquisition and release of locks for types
    * that implement `lock()` and `unlock()` methods. It supports both single-instance
    * and dual-instance locking with automatic deadlock prevention.
    *
    * When locking two instances, the guard ensures consistent lock ordering by only
    * acquiring the lock once if both references point to the same object.
    *
    * @tparam T The lockable type. Must provide:
    *           - `void lock() const noexcept`
    *           - `void unlock() const noexcept`
    *
    * @par Example Usage
    * @code
    * // Single instance lock
    * {
    *     util::SpinGuard<MyClass> guard(obj);
    *     // Critical section - obj is locked
    * } // obj is automatically unlocked
    *
    * // Dual instance lock
    * {
    *     util::SpinGuard<MyClass> guard(obj1, obj2);
    *     // Both obj1 and obj2 are locked (or just obj1 if obj1 == obj2)
    * } // Unlocked in reverse order
    * @endcode
    *
    * @attention
    * - This guard is **not reentrant**. Attempting to lock the same instance
    *   multiple times will result in deadlock.
    * - The guard is not movable or copyable to prevent accidental lock release.
    * - Locks are held for the entire lifetime of the guard object.
    */
    template<typename T>
    struct SpinGuard
    {
        T const &a_;
        T const &b_;

        /** @brief Constructs a guard for a single instance. */
        /**
        * @param v [in] The instance to lock.
        */
        explicit SpinGuard( T const &v ) noexcept
            : SpinGuard( v, v ) {}

        /** @brief Constructs a guard for two instances. */
        /**
        * @details
        * If both references point to the same object, only one lock is acquired.
        * Otherwise, both instances are locked in order (a, then b).
        *
        * @param a [in] The first instance to lock.
        * @param b [in] The second instance to lock.
        */
        explicit SpinGuard( T const &a, T const &b ) noexcept
            : a_( a ), b_( b )
        {
            // Note: To prevent deadlocks, only one if the same instance will be locked.
            if ( &a_ == &b_ ) { a_.lock();            }
            else              { a_.lock(); b_.lock(); }
        }

        /** @brief Destructor - unlocks in reverse order. */
        ~SpinGuard()
        {
            // Note: Unlock in reverse order.
            if ( &a_ == &b_ ) {              a_.unlock(); }
            else              { b_.unlock(); a_.unlock(); }
        }

        SpinGuard( SpinGuard const & ) = delete;
        SpinGuard &operator=( SpinGuard const & ) = delete;
        SpinGuard( SpinGuard && ) = delete;
        SpinGuard &operator=( SpinGuard && ) = delete;
    };

} // namespace util

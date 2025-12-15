#pragma once

#include <optional>
#include <memory>

/**
 * @brief Forward-declare FreeRTOS semaphore handle type.
 * @details
 * We purposely do not include the full FreeRTOS headers to avoid unwanted includes.
 * We use pointer aliases here to match the nature of opaque handles.
 */
// Implementation hidden by PIMPL below; header no longer exposes semaphore handle

class Mutex
{
public:
    /**
     * @brief Factory method to create a mutex.
     * @details
     * Constructs a RAII-managed wrapper for a FreeRTOS recursive mutex.
     * If creation of the underlying FreeRTOS mutex fails
     * (e.g. due to insufficient memory), this method returns \ref std::nullopt.
     * @return Created \c Mutex on success, or std::nullopt on failure.
     */
    static std::optional<Mutex> create( void );

    /**
     * @brief Destructor.
     * @details
     * Releases the underlying FreeRTOS mutex to avoid resource leaks.
     * Ensures RAII semantics: allocation in constructor, release in destructor.
     */
    ~Mutex( void ) noexcept;

    /**
     * @brief Copy constructor.
     * @details
     * Copy operations are disabled to avoid multiple instances managing
     * the same FreeRTOS mutex.
     */
    Mutex( const Mutex & ) noexcept = delete;

    /**
     * @brief Copy assignment operator.
     * @details
     * Copy operations are disabled to avoid multiple instances managing
     * the same FreeRTOS mutex.
     */
    Mutex &operator = ( const Mutex & ) noexcept = delete;

    /**
     * @brief Move constructor.
     * @details
     * Enabled only to allow construction into \ref std::optional.
     * Copy is disabled, so \c std::optional requires move.
     * Direct move usage in user code is not intended.
     */
    Mutex( Mutex && ) noexcept = default;

    /**
     * @brief Move assignment operator.
     * @details
     * Explicitly disabled to preserve RAII semantics.
     * Prevents reassigning an existing \c Mutex to a different FreeRTOS handle,
     * which would break resource ownership consistency.
     */
    Mutex &operator = ( Mutex && ) noexcept = delete;

private:
    Mutex( void ) noexcept = default;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

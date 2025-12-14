#include "mutex.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <memory>

struct Mutex::Impl
{
    SemaphoreHandle_t mutex = nullptr;
};

Mutex::~Mutex() noexcept
{
    if ( impl_ && impl_->mutex ) {
        vSemaphoreDelete( impl_->mutex );
        impl_->mutex = nullptr;
    }
}

std::optional<Mutex> Mutex::create(void)
{
    Mutex m;

    auto impl = std::make_unique<Impl>();
    impl->mutex = xSemaphoreCreateRecursiveMutex();

    if ( impl->mutex )
    {
        m.impl_ = std::move( impl );
        return std::optional<Mutex>( std::move( m ) );
    }
    else
    {
        return std::nullopt;
    }
}
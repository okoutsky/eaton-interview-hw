#pragma once

#include <functional>
#include <optional>

namespace hw::common
{

/**
 * @brief Class holding callback function.
 * Purpose of this class is to make work with callbacks cleaner and easier.
 *
 * @tparam HandlerSignature Signature of the callback function
 */
template <class HandlerSignature>
class handler_holder
{
public:
    handler_holder() = default;

    /**
     * @brief Assign callback function
     *
     * @param handler_ Callback function
     * @return handler_holder this
     */
    handler_holder& operator=(std::function<HandlerSignature> handler_)
    {
        _handler = std::move(handler_);
        return *this;
    }

    /**
     * @brief Call operator
     *
     * @tparam Args Arg pack
     * @param args_ Forwarded arguments
     */
    template <class... Args>
    void operator()(Args&&... args_)
    {
        if (_handler)
            (*_handler)(std::forward<Args>(args_)...);
    }

private:
    std::optional<std::function<HandlerSignature>> _handler;
};
}
#pragma once

#include <functional>
#include <optional>

namespace hw::common
{

template <class HandlerSignature>
class handler_holder
{
public:
    handler_holder() = default;

    handler_holder& operator=(std::function<HandlerSignature> handler_)
    {
        _handler = std::move(handler_);
        return *this;
    }

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
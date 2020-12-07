#pragma once


#include <boost/asio.hpp>

namespace hw::common
{

template <class AsyncClass>
class asynchronous : std::enable_shared_from_this<asynchronous<AsyncClass>>
{
public:
    asynchronous(boost::asio::io_context& ioc_)
        : _strand(ioc_)
    {}

    virtual ~asynchronous() = default;

    template <class Func, class... Args>
    void post_member_wrapper(Func func_, Args&&... args_)
    {
        boost::asio::post(_strand, std::bind(std::move(func_), static_cast<AsyncClass*>(this), std::forward<decltype(args_)>(args_)...));
    }

protected:
    boost::asio::io_context::strand _strand;
};
}
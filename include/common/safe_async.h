#pragma once


#include <boost/asio.hpp>

namespace hw::common
{

/**
 * @brief Class providing thread-safe invocation of Boost.Asio handlers
 * @tparam AsyncClass Asynchronous class
 */
template <class AsyncClass>
class safe_async : std::enable_shared_from_this<safe_async<AsyncClass>>
{
public:
    /**
     * @brief Constructor
     *
     * @param ioc_ Boost.Asio io_context
     */
    safe_async(boost::asio::io_context& ioc_)
        : _strand(ioc_)
    {}

    virtual ~safe_async() = default;

    /**
     * @brief Boost.Asio post @ref AsyncClass member function to synchronisation strand
     *
     * @tparam Func Function type to post
     * @tparam Args Arguments of the function
     * @param func_ Function to post
     * @param args_ Arguments
     */
    template <class Func, class... Args>
    void post_member_wrapper(Func func_, Args&&... args_)
    {
        boost::asio::post(_strand, std::bind(std::move(func_), static_cast<AsyncClass*>(this), std::forward<decltype(args_)>(args_)...));
    }

protected:
    boost::asio::io_context::strand _strand;
};
}
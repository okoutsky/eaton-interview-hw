#pragma once

#include <memory>

#include <boost/asio.hpp>

namespace hw::common
{

/**
 * @brief Class providing safe invocation of Boost.Asio handlers
 *
 * This class solves two problems associated with using Boost.Asio asynchronously:
 *
 * 1. Thread-safety:
 * When boost::asio::io_context::run() is called from multiple threads, there is no guarantee from which of the threads will any handler
 * be invoked. Handlers might even be invoked concurently. By associating certain handlers with some boost::asio::io_context::strand
 * object, Boost.Asio guarantees that these handlers will be invoked one after another, i.e. not concurently.
 * More on https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio/tutorial/tuttimer5.html.
 *
 * 2. Handlers bound to destructed objects:
 * Boost.Asio handlers are free functions invoked from boost::asio::io_context. If the handler is associated with a member function bound
 * to some parent object, it gets invoked regardless of whether the parent object still exists or not. If the object has already been destructed,
 * this usually leads to segmentation fault. The safe_async class provides functions preventing such situation. Both post_member_safe(...) and
 * wrap_member_safe(...) template functions create handlers containing weak_pointer of the parent object. When such handler gets invoked from
 * io_context, member function is invoked only if the weak pointer to parent object can locked, i.e. the object still exists.
 *
 * @tparam AsyncClass Asynchronous class
 */
template <class AsyncClass>
class safe_async : public std::enable_shared_from_this<safe_async<AsyncClass>>
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
    void post_member_safe(Func func_, Args&&... args_)
    {
        auto to_post = [weak_this = std::weak_ptr<AsyncClass>(static_pointer_cast<AsyncClass>(this->shared_from_this())),
                        func{std::move(func_)}](auto&&... args_) -> void {
            if (auto locked = weak_this.lock())
            {
                (locked.get()->*func)(std::forward<Args>(args_)...);
            }
        };

        boost::asio::post(_strand, std::bind(std::move(to_post), std::forward<Args>(args_)...));
    }

    /**
     * @brief Boost.Asio wrap @ref AsyncClass member function to synchronisation strand
     *
     * @tparam Func Function type to post
     * @param func_ Function to post
     */
    template <class Func>
    auto wrap_member_safe(Func func_)
    {
        return [weak_this = std::weak_ptr<AsyncClass>(static_pointer_cast<AsyncClass>(this->shared_from_this())), func{std::move(func_)}](auto&&... args_) {
            if (auto locked = weak_this.lock())
            {
                locked->post_member_safe(std::move(func), std::forward<decltype(args_)>(args_)...);
            }
        };
    }

protected:
    boost::asio::io_context::strand _strand;
};
}
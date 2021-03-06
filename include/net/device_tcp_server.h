#pragma once

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/safe_async.h>
#include <common/types.h>
#include <device_control_messages/messages.h>
#include <net/device_tcp_connection.h>
#include <net/types.h>

namespace hw::net
{

/**
 * @brief TCP server listening for connection from devices
 *
 * @tparam MessageSerializer Type of message serializer/deserializer
 */
template <class MessageSerializer>
class device_tcp_server : public common::safe_async<device_tcp_server<MessageSerializer>>
{
public:
    /**
     * @brief Constructor
     *
     * @param ioc_ Boost.Asio io_context
     */
    device_tcp_server(boost::asio::io_context& ioc_)
        : common::safe_async<device_tcp_server<MessageSerializer>>(ioc_)
        , _acceptor(ioc_)
        , _sock(ioc_)
    {}

    /**
     * @brief Start listening
     *
     * @param ip_address_ IP address to listen on
     * @param tcp_port_ TCP port to listen on
     */
    void listen(const ip_address_t& ip_address_, port_t tcp_port_)
    {
        this->post_member_safe(&device_tcp_server<MessageSerializer>::listen_impl, std::move(ip_address_), tcp_port_);
    }

public:
    //! Callback triggered when device control message is received. Callback parameter: deserialized device control message.
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;
    //! Callback triggered when error occurs.
    common::handler_holder<void()> on_error;

private:
    // Listen internal implementation - must be invoked from within strand context
    void listen_impl(const ip_address_t& ip_address_, port_t tcp_port_)
    {
        if (_acceptor.is_open())
            return;

        boost::system::error_code ec;
        auto ip = boost::asio::ip::make_address(ip_address_, ec);

        if (ec)
        {
            std::cerr << me() << "Error forming IP address. EC(" << ec << ")" << std::endl;
            on_error();
            return;
        }

        boost::asio::ip::tcp::endpoint ep(ip, tcp_port_);

        _acceptor.open(ep.protocol(), ec);
        if (ec)
        {
            std::cerr << me() << "Error opening acceptor. EC(" << ec << ")" << std::endl;
            on_error();
        }

        _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            std::cerr << me() << "Error seting acceptor options. EC(" << ec << ")" << std::endl;
            on_error();
            return;
        }

        _acceptor.bind(ep, ec);
        if (ec)
        {
            std::cerr << me() << "Error binding acceptor. EC(" << ec << ")" << std::endl;
            on_error();
            return;
        }

        _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            std::cerr << me() << "Error during listen. EC(" << ec << ")" << std::endl;
            on_error();
            return;
        }

        _acceptor.async_accept(_sock, this->wrap_member_safe(&device_tcp_server<MessageSerializer>::handle_accept));
    }

    // Handler called when new TCP connection is accepted
    void handle_accept(boost::system::error_code ec_)
    {
        if (ec_)
        {
            if (ec_ != boost::asio::error::operation_aborted)
            {
                std::cerr << me() << "Error during listen. EC(" << ec_ << ")" << std::endl;
                on_error();
            }
            return;
        }

        auto conn        = std::make_shared<device_tcp_connection<MessageSerializer>>(this->_strand.context(), std::move(_sock));
        conn->on_message = this->wrap_member_safe(&device_tcp_server<MessageSerializer>::handle_conn_message);
        conn->on_close   = this->wrap_member_safe(&device_tcp_server<MessageSerializer>::remove_connection);
        conn->on_error   = this->wrap_member_safe(&device_tcp_server<MessageSerializer>::remove_connection);

        conn->start_receive();
        auto id = conn->get_connection_id();
        _connections.emplace(id, std::move(conn));

        _acceptor.async_accept(_sock, this->wrap_member_safe(&device_tcp_server<MessageSerializer>::handle_accept));
    }

    // Callback for connection close and error
    void remove_connection(size_t id_) { _connections.erase(id_); }

    // Callback for connection message
    void handle_conn_message(device_control_messages::device_message_type message_) { on_message(std::move(message_)); }

    // For logging purposes
    std::string me() const { return "[device_tcp_server] "; }

private:
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _sock;
    std::unordered_map<size_t, std::shared_ptr<device_tcp_connection<MessageSerializer>>> _connections;
};
}
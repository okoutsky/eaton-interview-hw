#pragma once

#include <boost/asio.hpp>

#include <common/asynchronous.h>
#include <common/handler_holder.h>
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
class device_tcp_server : public common::asynchronous<device_tcp_server<MessageSerializer>>
{
public:
    /**
     * @brief Constructor
     *
     * @param ioc_ Boost.Asio io_context
     */
    device_tcp_server(boost::asio::io_context& ioc_)
        : common::asynchronous<device_tcp_server<MessageSerializer>>(ioc_)
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
        this->post_member_wrapper(&device_tcp_server<MessageSerializer>::listen_impl, std::move(ip_address_), tcp_port_);
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
            // TODO: log error creating address
            on_error();
            return;
        }

        boost::asio::ip::tcp::endpoint ep(ip, tcp_port_);

        _acceptor.open(ep.protocol(), ec);
        if (ec)
        {
            // TODO: log error
            on_error();
        }

        _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            // TODO: log error
            on_error();
            return;
        }

        _acceptor.bind(ep, ec);
        if (ec)
        {
            // TODO: log error binding
            on_error();
            return;
        }

        _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            // TODO: log error listening
            on_error();
            return;
        }

        _acceptor.async_accept(_sock, [this](auto ec_) { this->post_member_wrapper(&device_tcp_server<MessageSerializer>::handle_accept, ec_); });
    }

    // Handler called when new TCP connection is accepted
    void handle_accept(boost::system::error_code ec_)
    {
        if (ec_)
        {
            if (ec_ != boost::asio::error::operation_aborted)
                on_error();
            return;
        }
        auto conn        = std::make_shared<device_tcp_connection<MessageSerializer>>(std::move(_sock));
        conn->on_message = [this](auto msg_) { on_message(std::move(msg_)); };
        conn->on_close   = [this](auto id_) {
            /* TODO: Log connection close*/
            this->post_member_wrapper(&device_tcp_server<MessageSerializer>::remove_connection, id_);
        };
        conn->on_error = [this](auto id_) {
            /* TODO: Log connection error*/
            this->post_member_wrapper(&device_tcp_server<MessageSerializer>::remove_connection, id_);
        };

        conn->start_receive();
        auto id = conn->get_connection_id();
        _connections.emplace(id, std::move(conn));

        _acceptor.async_accept(_sock, [this](auto ec_) { this->post_member_wrapper(&device_tcp_server<MessageSerializer>::handle_accept, ec_); });
    }

    // Callback for connection close and error
    void remove_connection(size_t id_) { _connections.erase(id_); }

private:
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _sock;
    std::unordered_map<size_t, std::shared_ptr<device_tcp_connection<MessageSerializer>>> _connections;
};
}
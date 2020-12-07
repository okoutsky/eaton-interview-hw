#pragma once

#include <vector>

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
 * @brief TCP client connecting to device monitoring center.
 * Providing interface for transferring device control messages over TCP.
 *
 * @tparam MessageSerializer Type of message serializer/deserializer
 */
template <class MessageSerializer>
class device_tcp_client : public common::safe_async<device_tcp_client<MessageSerializer>>
{
public:
    /**
     * @brief Constructor
     *
     * @param ioc_ Boost.Asio io_context
     */
    device_tcp_client(boost::asio::io_context& ioc_)
        : common::safe_async<device_tcp_client<MessageSerializer>>(ioc_)
        , _sock(ioc_)
    {}

    /**
     * @brief Connect to device monitoring center
     *
     * @param ip_address_ IP address of the center
     * @param tcp_port_ TCP port of the center
     */
    void connect(const ip_address_t& ip_address_, port_t tcp_port_)
    {
        this->post_member_wrapper(&device_tcp_client::connect_impl, std::move(ip_address_), tcp_port_);
    }

    /**
     * @brief Send device control message to device monitoring center
     *
     * @param message_ Message to send
     */
    void send(device_control_messages::device_message_type message_) { this->post_member_wrapper(&device_tcp_client::send_impl, std::move(message_)); }

public:
    //! Callback triggered when client successfully connects
    common::handler_holder<void()> on_connect;
    //! Callback triggered when error occurs
    common::handler_holder<void()> on_error;
    //! Callback triggered when connection is closed
    common::handler_holder<void()> on_close;
    //! Callback triggered when device control message is received. Callback parameter: deserialized device control message
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;

private:
    // Send internal implementation - must be invoked from within strand context
    void send_impl(device_control_messages::device_message_type message_)
    {
        if (!_connection)
        {
            // TODO: log error
            return;
        }

        _connection->send(std::move(message_));
    }

    // Connect internal implementation - must be invoked from within strand context
    void connect_impl(const ip_address_t& ip_address_, port_t tcp_port_)
    {
        if (_connected)
        {
            // TODO: log error
        }

        boost::system::error_code ec;
        auto ip = boost::asio::ip::make_address(ip_address_, ec);

        if (ec)
        {
            // TODO: log error creating address
            on_error();
            return;
        }

        boost::asio::ip::tcp::endpoint ep(ip, tcp_port_);
        _sock.async_connect(ep,
                            [this](boost::system::error_code ec_) { this->post_member_wrapper(&device_tcp_client<MessageSerializer>::handle_connected, ec_); });
    }

    // Handler called when TCP connection is established
    void handle_connected(boost::system::error_code ec_)
    {
        if (ec_)
        {
            // TOOD: log connecting error
            if (ec_ != boost::asio::error::operation_aborted)
                on_error();
            return;
        }

        _connection.emplace(std::move(_sock));
        _connection->on_message = [this](auto msg_) { on_message(std::move(msg_)); };
        _connection->on_close   = [this](auto) { on_close(); };
        _connection->on_error   = [this](auto) { on_error(); };

        on_connect();
    }

private:
    boost::asio::ip::tcp::socket _sock;
    std::optional<device_tcp_connection<MessageSerializer>> _connection;
    std::vector<hw::common::byte_t> _recv_buffer;
    std::vector<hw::common::byte_t> _unprocessed_recv_data;
    bool _connected{false};
};
}
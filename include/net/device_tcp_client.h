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
        this->post_member_safe(&device_tcp_client::connect_impl, std::move(ip_address_), tcp_port_);
    }

    /**
     * @brief Send device control message to device monitoring center
     *
     * @param message_ Message to send
     */
    void send(device_control_messages::device_message_type message_) { this->post_member_safe(&device_tcp_client::send_impl, std::move(message_)); }

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
            std::cerr << me() << "Send refused. REASON(not connected)" << std::endl;
            return;
        }

        _connection->send(std::move(message_));
    }

    // Connect internal implementation - must be invoked from within strand context
    void connect_impl(const ip_address_t& ip_address_, port_t tcp_port_)
    {
        if (_connected)
        {
            std::cerr << me() << "Connect refused. REASON(already connected)" << std::endl;
            return;
        }

        boost::system::error_code ec;
        auto ip = boost::asio::ip::make_address(ip_address_, ec);

        if (ec)
        {
            std::cerr << me() << "Error forming IP address. EC(" << ec << ")" << std::endl;
            on_error();
            return;
        }

        boost::asio::ip::tcp::endpoint ep(ip, tcp_port_);
        _sock.async_connect(ep, this->wrap_member_safe(&device_tcp_client<MessageSerializer>::handle_connected));
    }

    // Handler called when TCP connection is established
    void handle_connected(boost::system::error_code ec_)
    {
        if (ec_)
        {
            std::cerr << me() << "Error during connecting. EC(" << ec_ << ")" << std::endl;
            if (ec_ != boost::asio::error::operation_aborted)
                on_error();
            return;
        }

        _connection             = std::make_shared<device_tcp_connection<MessageSerializer>>(this->_strand.context(), std::move(_sock));
        _connection->on_message = this->wrap_member_safe(&device_tcp_client<MessageSerializer>::handle_conn_message);
        _connection->on_close   = this->wrap_member_safe(&device_tcp_client<MessageSerializer>::handle_conn_close);
        _connection->on_error   = this->wrap_member_safe(&device_tcp_client<MessageSerializer>::handle_conn_error);

        on_connect();
    }

    // Handlers for connection callbacks
    void handle_conn_message(device_control_messages::device_message_type message_) { on_message(std::move(message_)); }
    void handle_conn_close(size_t) { on_close(); }
    void handle_conn_error(size_t) { on_error(); }

    // For logging purposes
    std::string me() const { return "[device_tcp_client] "; }

private:
    boost::asio::ip::tcp::socket _sock;
    std::shared_ptr<device_tcp_connection<MessageSerializer>> _connection;
    std::vector<hw::common::byte_t> _recv_buffer;
    std::vector<hw::common::byte_t> _unprocessed_recv_data;
    bool _connected{false};
};
}
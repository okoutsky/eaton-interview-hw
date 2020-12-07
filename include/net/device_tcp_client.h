#pragma once

#include <cassert>
#include <vector>

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/types.h>
#include <device_control_messages/message_json_coverter.h>
#include <device_control_messages/messages.h>
#include <net/device_tcp_connection.h>


namespace hw::net
{
template <class MessageSerializer>
class device_tcp_client
{
public:
    device_tcp_client(boost::asio::io_context& ioc_)
        : _sock(ioc_)
    {}

    void connect(const common::ip_address_t& ip_address_, common::port_t tcp_port_)
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
        _sock.async_connect(ep, [this](boost::system::error_code ec_) { handle_connected(ec_); });
    }

    void send(device_control_messages::device_message_type message_)
    {
        if (!_connection)
        {
            // TODO: log error
            return;
        }

        _connection->send(std::move(message_));
    }

public:
    common::handler_holder<void()> on_connect;
    common::handler_holder<void()> on_error;
    common::handler_holder<void()> on_close;
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;

private:
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
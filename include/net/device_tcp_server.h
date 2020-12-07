#pragma once

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/types.h>
#include <device_control_messages/messages.h>
#include <net/device_tcp_connection.h>

namespace hw::net
{

template <class MessageSerializer>
class device_tcp_server
{
public:
    device_tcp_server(boost::asio::io_context& ioc_)
        : _acceptor(ioc_)
        , _sock(ioc_)
    {}

    void listen(const common::ip_address_t& ip_address_, common::port_t tcp_port_)
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

        _acceptor.async_accept(_sock, [this](auto ec_) { handle_accept(ec_); });
    }

public:
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;
    common::handler_holder<void()> on_error;

private:
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
            _connections.erase(id_);
            std::cout << "conn close" << std::endl;
        }; // TODO
        conn->on_error = [this](auto id_) {
            _connections.erase(id_);
            std::cout << "conn error" << std::endl;
        }; // TODO

        conn->start_receive();
        auto id = conn->get_connection_id();
        _connections.emplace(id, std::move(conn));

        _acceptor.async_accept(_sock, [this](auto ec_) { handle_accept(ec_); });
    }

private:
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _sock;
    std::unordered_map<size_t, std::shared_ptr<device_tcp_connection<MessageSerializer>>> _connections;
};
}
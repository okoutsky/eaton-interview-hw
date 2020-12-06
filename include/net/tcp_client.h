#pragma once

#include <cassert>

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/types.h>

namespace hw::net
{
class tcp_client
{
public:
    tcp_client(boost::asio::io_context& ioc_)
        : _ioc(ioc_)
    {}

    void connect(const common::ip_address_t& ip_address_, common::port_t tcp_port_)
    {
        boost::system::error_code ec;
        auto ip = boost::asio::ip::make_address(ip_address_, ec);

        if (ec)
        {
            // TODO: log error creating address
            on_error(ec);
            return;
        }

        boost::asio::ip::tcp::endpoint ep(ip, tcp_port_);
        _sock.async_connect(ep, [this](boost::system::error_code ec_) {
            if (ec_)
            {
                // TOOD: log connecting error
                on_error();
                return;
            }
            on_connect();
            _sock.async_receive(boost::asio::buffer(_recv_buffer), [this](boost::system::error_code ec_, size_t bytes_received_) {
                assert(bytes_received_ <= _recv_buffer.size());

                if (ec_)
                {
                    if (ec_ == boost::asio::error::eof)
                        on_close();
                    else if (ec_ != boost::asio::error::operation_aborted)
                        on_error();

                    return;
                }

                if (ec_ == boost::asio::error::eof)
                {
                    on_close();
                    return;
                }
                if (ec_)
                {
                    on_error();
                    return;
                }

                on_read(std::vector<common::byte_t>(_recv_buffer.begin(), _recv_buffer.begin() + bytes_received_));
            });
        });
    }

public:
    common::handler_holder<void()> on_connect;
    common::handler_holder<void()> on_error;
    common::handler_holder<void()> on_close;
    common::handler_holder<void(size_t)> on_send;
    common::handler_holder<void(std::vector<common::byte_t>)> on_read;

private:
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::socket _sock;
    std::vector<common::byte_t> _data_to_send;
    std::vector<common::byte_t> _recv_buffer;
};
}
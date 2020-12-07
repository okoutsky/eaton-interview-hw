#pragma once

#include <atomic>
#include <deque>

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/types.h>
#include <device_control_messages/device_message_type.h>
#include <device_control_messages/message_types.h>


namespace hw::net
{
template <class MessageSerializer>
class device_tcp_connection
{
public:
    device_tcp_connection(boost::asio::ip::tcp::socket sock_)
        : _sock(std::move(sock_))
        , _recv_buffer(recv_buffer_len)
        , _connection_id(generate_id())
    {}

    device_tcp_connection(const device_tcp_connection&) = delete;

    device_tcp_connection(device_tcp_connection&& other_conn_)
        : _sock(std::move(other_conn_._sock))
    {}

    void start_receive()
    {
        _sock.async_receive(boost::asio::buffer(_recv_buffer), [this](auto ec_, auto bytes_read_) { handle_receive(ec_, bytes_read_); });
    }

    void send(device_control_messages::device_message_type message_)
    {
        _messages_to_send.push_back(std::move(message_));
        if (_messages_to_send.size() == 1)
            send_next_message();
    }

    size_t get_connection_id() const { return _connection_id; }

public:
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;
    common::handler_holder<void(size_t)> on_error;
    common::handler_holder<void(size_t)> on_close;

private:
    static constexpr size_t recv_buffer_len = 1024;

private:
    void handle_receive(boost::system::error_code ec_, size_t bytes_read_)
    {
        if (ec_)
        {
            if (ec_ == boost::asio::error::eof)
                on_close(_connection_id);
            else if (ec_ != boost::asio::error::operation_aborted)
                on_error(_connection_id);
            return;
        }

        std::copy(_recv_buffer.begin(), _recv_buffer.begin() + bytes_read_, std::back_inserter(_unprocessed_recv_data));
        auto [message, bytes_read] = MessageSerializer::deserialize(_unprocessed_recv_data);
        while (message)
        {
            assert(bytes_read != 0);
            on_message(std::move(*message));
            _unprocessed_recv_data.erase(_unprocessed_recv_data.begin(), _unprocessed_recv_data.begin() + bytes_read);

            std::tie(message, bytes_read) = MessageSerializer::deserialize(_unprocessed_recv_data);
        }

        _sock.async_receive(boost::asio::buffer(_recv_buffer), [this](auto ec_, auto bytes_read_) { handle_receive(ec_, bytes_read_); });
    }

    void send_next_message()
    {
        auto fill_sending_buffer_visitor = [this](auto msg_) { _sending_buffer = MessageSerializer::serialize(msg_); };

        std::visit(fill_sending_buffer_visitor, _messages_to_send.front());

        boost::asio::async_write(_sock, boost::asio::buffer(_sending_buffer), [this](auto ec_, auto) { handle_message_sent(ec_); });
        _messages_to_send.pop_front();
    }

    void handle_message_sent(boost::system::error_code ec_)
    {
        if (ec_)
        {
            if (ec_ != boost::asio::error::operation_aborted)
                on_error(_connection_id);
            return;
        }

        if (!_messages_to_send.empty())
            send_next_message();
    }

    size_t generate_id()
    {
        static std::atomic<size_t> id{0};
        return id++;
    }

private:
    boost::asio::ip::tcp::socket _sock;
    std::deque<device_control_messages::device_message_type> _messages_to_send;
    std::vector<hw::common::byte_t> _sending_buffer;
    std::vector<common::byte_t> _recv_buffer;
    std::vector<common::byte_t> _unprocessed_recv_data;

    const size_t _connection_id;
};
}
#pragma once

#include <atomic>
#include <deque>
#include <iostream>

#include <boost/asio.hpp>

#include <common/handler_holder.h>
#include <common/safe_async.h>
#include <common/types.h>
#include <device_control_messages/messages.h>

namespace hw::net
{
/**
 * @brief Class transferring device control messages over TCP connection
 *
 * @tparam MessageSerializer Type of message serializer/deserializer
 */
template <class MessageSerializer>
class device_tcp_connection : public common::safe_async<device_tcp_connection<MessageSerializer>>
{
public:
    /**
     * @brief Constructor
     *
     * @param sock_ Connected TCP socket
     */
    device_tcp_connection(boost::asio::io_context& ioc_, boost::asio::ip::tcp::socket sock_)
        : common::safe_async<device_tcp_connection<MessageSerializer>>(ioc_)
        , _sock(std::move(sock_))
        , _recv_buffer(recv_buffer_len)
        , _connection_id(generate_id())
    {}

    /** @brief Start receiving messages */
    void start_receive() { this->post_member_safe(&device_tcp_connection<MessageSerializer>::start_receive_impl); }

    /**
     * @brief Send device control message
     *
     * @param message_ Message to send
     */
    void send(device_control_messages::device_message_type message_)
    {
        this->post_member_safe(&device_tcp_connection<MessageSerializer>::send_impl, std::move(message_));
    }

    /**
     * @brief Get unique connection ID
     *
     * @return ID
     */
    size_t get_connection_id() const { return _connection_id; }

public:
    //! Callback triggered when new device control message is received on the connection. Callback parameter: deserialized message.
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;
    //! Callback triggered when error occurs. Callback type: connection ID.
    common::handler_holder<void(size_t)> on_error;
    //! Callback triggered when connection is closed. Callback type: connection ID.
    common::handler_holder<void(size_t)> on_close;

private:
    // Data is read in buffers of this size
    static constexpr size_t recv_buffer_len = 1024;

private:
    // Start receive internal implementation - must be invoked from within strand context
    void start_receive_impl() { _sock.async_receive(boost::asio::buffer(_recv_buffer), this->wrap_member_safe(&device_tcp_connection::handle_receive)); }

    // Send internal implementaiton - must be invoked from within strand context
    void send_impl(device_control_messages::device_message_type message_)
    {
        _messages_to_send.push_back(std::move(message_));
        if (_messages_to_send.size() == 1)
            send_next_message();
    }

    // Handler called when data is received on socket
    void handle_receive(boost::system::error_code ec_, size_t bytes_read_)
    {
        if (ec_)
        {
            if (ec_ == boost::asio::error::eof)
            {
                on_close(_connection_id);
            }
            else if (ec_ != boost::asio::error::operation_aborted)
            {
                std::cerr << me() << "Error receiving. EC(" << ec_ << ")" << std::endl;
                on_error(_connection_id);
            }

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

        _sock.async_receive(boost::asio::buffer(_recv_buffer), this->wrap_member_safe(&device_tcp_connection<MessageSerializer>::handle_receive));
    }

    // Send next message in queue
    void send_next_message()
    {
        _sending_buffer = MessageSerializer::serialize(_messages_to_send.front());
        boost::asio::async_write(
            _sock, boost::asio::buffer(_sending_buffer), this->wrap_member_safe(&device_tcp_connection<MessageSerializer>::handle_message_sent));

        _messages_to_send.pop_front();
    }

    // Handler called when data is sent to socket
    void handle_message_sent(boost::system::error_code ec_, size_t)
    {
        if (ec_)
        {
            if (ec_ != boost::asio::error::operation_aborted)
            {
                std::cerr << me() << "Error sending message. EC(" << ec_ << ")" << std::endl;
                on_error(_connection_id);
            }
            return;
        }

        if (!_messages_to_send.empty())
            send_next_message();
    }

    // Generate unique connection ID
    size_t generate_id()
    {
        static std::atomic<size_t> id{0};
        return id++;
    }

    std::string me() const { return std::string{"[device_tcp_connection/"} + std::to_string(_connection_id) + std::string{"] "}; }

private:
    boost::asio::ip::tcp::socket _sock;
    std::deque<device_control_messages::device_message_type> _messages_to_send;
    std::vector<hw::common::byte_t> _sending_buffer;
    std::vector<common::byte_t> _recv_buffer;
    std::vector<common::byte_t> _unprocessed_recv_data;
    const size_t _connection_id;
};
}
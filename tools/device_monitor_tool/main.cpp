
#include <iostream>
#include <vector>

#include <boost/asio.hpp>

#include <common/types.h>
#include <device_control_messages/message_json_coverter.h>
#include <net/device_tcp_server.h>

int main()
{

    boost::asio::io_context ioc;

    hw::net::device_tcp_server<hw::device_control_messages::json_serializer> server(ioc);

    server.on_error = [] { std::cout << "Error" << std::endl; };

    server.on_message = [](auto msg_) {
        auto printing_visitor = [](auto m_) { std::cout << m_.as_string() << std::endl; };

        std::visit(printing_visitor, msg_);
    };
    server.listen("127.0.0.1", 12345);

    ioc.run();

    return 0;
}
#include <catch2/catch.hpp>

#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include <device_control_messages/message_json_coverter.h>
#include <device_control_messages/messages.h>
#include <net/device_tcp_client.h>
#include <net/device_tcp_server.h>

TEST_CASE("Reporting messages using TCP")
{
    hw::device_control_messages::measurement meas_msg("device");
    meas_msg.temperature_sensors = std::vector<uint16_t>{1, 2, 3};
    meas_msg.fans_speed          = std::vector<uint8_t>{1};
    hw::device_control_messages::error error_msg("device", hw::device_control_messages::error::error_type::exploded);

    const hw::net::ip_address_t ip = "127.0.0.1";
    const hw::net::port_t port     = 12345;

    bool client_error{false};
    bool client_connected{false};
    bool client_close{false};
    bool server_error{false};
    bool server_msg_received{false};

    bool expect_client_error{false};
    bool expect_client_connected{false};
    bool expect_client_close{false};
    bool expect_server_error{false};
    bool expect_server_msg_received{false};

    boost::asio::io_context ioc;

    auto client = std::make_shared<hw::net::device_tcp_client<hw::device_control_messages::json_serializer>>(ioc);
    auto server = std::make_shared<hw::net::device_tcp_server<hw::device_control_messages::json_serializer>>(ioc);

    client->on_error = [&] { client_error = true; };
    client->on_close = [&] { client_close = true; };
    server->on_error = [&] { server_error = true; };

    SECTION("TCP client invalid server IP")
    {
        expect_client_error = true;
        client->connect("1.2.3", port);
    }

    SECTION("TCP client different server IP")
    {
        expect_client_error = true;
        client->connect("1.2.3.5", port);
    }

    SECTION("TCP client different server port")
    {
        expect_client_error = true;
        client->connect(ip, port + 1);
    }

    SECTION("measurement message transfer")
    {
        expect_server_msg_received = true;
        expect_client_connected    = true;

        client->on_connect = [&] {
            client_connected = true;
            client->send(meas_msg);
        };

        server->on_message = [&](auto msg_) {
            server_msg_received = true;
            hw::device_control_messages::measurement m;
            REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::measurement>(msg_));
            REQUIRE(m.device_name == meas_msg.device_name);
            REQUIRE(m.message_type == meas_msg.message_type);
            REQUIRE(m.temperature_sensors == meas_msg.temperature_sensors);
            REQUIRE(m.fans_speed == meas_msg.fans_speed);
        };
    }

    SECTION("error message transfer")
    {
        expect_server_msg_received = true;
        expect_client_connected    = true;

        client->on_connect = [&] {
            client_connected = true;
            client->send(error_msg);
        };
        server->on_message = [&](auto msg_) {
            server_msg_received = true;
            hw::device_control_messages::error m;
            REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::error>(msg_));
            REQUIRE(m.device_name == error_msg.device_name);
            REQUIRE(m.message_type == error_msg.message_type);
            REQUIRE(m.err_type == error_msg.err_type);
        };
    }

    std::vector<hw::device_control_messages::device_message_type> received_messages;
    SECTION("multiple messages")
    {
        expect_server_msg_received = true;
        expect_client_connected    = true;

        client->on_connect = [&] {
            client_connected = true;
            client->send(error_msg);
            client->send(meas_msg);
            client->send(meas_msg);
            client->send(error_msg);
        };
        server->on_message = [&](auto msg_) {
            received_messages.push_back(std::move(msg_));
            if (received_messages.size() == 4)
            {
                server_msg_received = true;
                for (size_t i = 0; i < received_messages.size(); i++)
                {
                    if (i == 1 | i == 2)
                    {
                        hw::device_control_messages::measurement m;
                        REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::measurement>(received_messages[i]));
                        REQUIRE(m.device_name == meas_msg.device_name);
                        REQUIRE(m.message_type == meas_msg.message_type);
                        REQUIRE(m.temperature_sensors == meas_msg.temperature_sensors);
                        REQUIRE(m.fans_speed == meas_msg.fans_speed);
                    }
                    else
                    {
                        hw::device_control_messages::error m;
                        REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::error>(received_messages[i]));
                        REQUIRE(m.device_name == error_msg.device_name);
                        REQUIRE(m.message_type == error_msg.message_type);
                        REQUIRE(m.err_type == error_msg.err_type);
                    }
                }
            }
        };
    }

    server->listen(ip, port);
    client->connect(ip, port);

    auto t = std::thread([&ioc] { ioc.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    client.reset();
    server.reset();
    t.join();

    REQUIRE(client_connected == expect_client_connected);
    REQUIRE(server_msg_received == expect_server_msg_received);
    REQUIRE(client_error == expect_client_error);
    REQUIRE(client_close == expect_client_close);
    REQUIRE(server_error == expect_server_error);
}
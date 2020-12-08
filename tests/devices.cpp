#include <catch2/catch.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include <devices/file_reading_device.h>

bool create_sensor_file(size_t file_value_, const std::string& file_path_)
{
    std::ofstream file(file_path_);
    if (!file)
        return false;

    file << std::to_string(file_value_);
    return true;
}

bool remove_sensor_file(const std::string& file_path_)
{
    return std::filesystem::remove(file_path_);
}

TEST_CASE("Correct reports")
{
    const std::string device_name = "test_device";
    const uint16_t temp_1         = 1234;
    const uint16_t temp_2         = 9876;
    const uint8_t fan_1           = 100;

    bool on_message_called{false};

    REQUIRE(create_sensor_file(temp_1, std::to_string(temp_1)));
    REQUIRE(create_sensor_file(temp_2, std::to_string(temp_2)));
    REQUIRE(create_sensor_file(fan_1, std::to_string(fan_1)));

    boost::asio::io_context ioc;
    std::shared_ptr<hw::devices::file_reading_device> device;

    SECTION("no files")
    {
        device             = std::make_shared<hw::devices::file_reading_device>(device_name, ioc, 100, std::vector<std::string>{}, std::vector<std::string>{});
        device->on_message = [&](auto msg_) {
            on_message_called = true;
            hw::device_control_messages::measurement m;
            REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::measurement>(msg_));
            REQUIRE(m.device_name == device_name);
            REQUIRE(m.message_type == hw::device_control_messages::message_type::measurement);
            REQUIRE(m.temperature_sensors == std::vector<uint16_t>{});
            REQUIRE(m.fans_speed == std::vector<uint8_t>{});
        };
        device->start();
    }

    SECTION("all files")
    {
        device = std::make_shared<hw::devices::file_reading_device>(
            device_name, ioc, 100, std::vector<std::string>{std::to_string(temp_1), std::to_string(temp_2)}, std::vector<std::string>{std::to_string(fan_1)});
        device->on_message = [&](auto msg_) {
            on_message_called = true;
            hw::device_control_messages::measurement m;
            REQUIRE_NOTHROW(m = std::get<hw::device_control_messages::measurement>(msg_));
            REQUIRE(m.device_name == device_name);
            REQUIRE(m.message_type == hw::device_control_messages::message_type::measurement);
            REQUIRE(m.temperature_sensors == std::vector<uint16_t>{temp_1, temp_2});
            REQUIRE(m.fans_speed == std::vector<uint8_t>{fan_1});
        };
        device->start();
    }

    auto t = std::thread([&ioc] { ioc.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    device.reset();
    t.join();

    REQUIRE(on_message_called);

    REQUIRE(remove_sensor_file(std::to_string(temp_1)));
    REQUIRE(remove_sensor_file(std::to_string(temp_2)));
    REQUIRE(remove_sensor_file(std::to_string(fan_1)));
}
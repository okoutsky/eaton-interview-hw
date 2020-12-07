#pragma once

#include <iostream>
#include <sstream>
#include <tuple>

#include <nlohmann/json.hpp>

#include <device_control_messages/device_message_type.h>

namespace hw::device_control_messages
{

namespace json_converter
{

template <class MessageType>
nlohmann::json serialize_to_json(const MessageType& message_)
{
    return nlohmann::json(message_);
}

template <class MessageType>
std::optional<MessageType> deserialize_from_json(const nlohmann::json& json_)
{
    try
    {
        MessageType message;
        json_.get_to(message);
        return message;
    }
    catch (const nlohmann::json::exception& e_)
    {
        return std::nullopt;
    }
}
} // namespace json_converter

namespace keys
{
constexpr char device_name[]         = "device_name";
constexpr char message_type[]        = "message_type";
constexpr char temperature_sensors[] = "temperature_sensors";
constexpr char fans_speed[]          = "fans_speed";
}

NLOHMANN_JSON_SERIALIZE_ENUM(device_control_messages::message_type,
                             {
                                 {device_control_messages::message_type::measurement, "measurement"},
                                 {device_control_messages::message_type::error, "error"},
                             })

inline void to_json(nlohmann::json& json_, const device_control_messages::measurement& measurement_)
{
    json_[keys::device_name]         = measurement_.device_name;
    json_[keys::message_type]        = measurement_.message_type;
    json_[keys::temperature_sensors] = nlohmann::json::array();
    for (auto temp : measurement_.temperature_sensors)
    {
        json_[keys::temperature_sensors].push_back(temp);
    }
    json_[keys::fans_speed] = nlohmann::json::array();
    for (auto speed : measurement_.fans_speed)
    {
        json_[keys::fans_speed].push_back(speed);
    }
}

inline void from_json(const nlohmann::json& json_, device_control_messages::measurement& measurement_)
{
    json_.at(keys::device_name).get_to(measurement_.device_name);
    json_.at(keys::message_type).get_to(measurement_.message_type);
    json_.at(keys::temperature_sensors).get_to(measurement_.temperature_sensors);
    json_.at(keys::fans_speed).get_to(measurement_.fans_speed);
}

inline void to_json(nlohmann::json& json_, const device_control_messages::error& error_)
{
    json_[keys::device_name]  = error_.device_name;
    json_[keys::message_type] = error_.message_type;
    // TODO: will have error code
}

inline void from_json(const nlohmann::json& json_, device_control_messages::error& error_)
{
    json_.at(keys::device_name).get_to(error_.device_name);
    json_.at(keys::message_type).get_to(error_.message_type);
}

class json_serializer
{
public:
    template <class MessageType>
    static std::vector<common::byte_t> serialize(const MessageType& message_)
    {
        auto str_json = json_converter::serialize_to_json(message_).dump();
        std::vector<common::byte_t> buffer(str_json.size());
        std::copy(str_json.begin(), str_json.end(), buffer.begin());
        return buffer;
    }

    static std::tuple<std::optional<device_message_type>, size_t> deserialize(std::vector<common::byte_t> data_)
    {
        try
        {
            std::stringstream ss;
            for (auto byte : data_)
            {
                ss << byte;
            }

            nlohmann::json json;
            ss >> json;
            std::string rest;
            ss >> rest;

            message_type msg_type;
            json[keys::message_type].get_to(msg_type);

            if (msg_type == message_type::error)
                return std::make_tuple(json_converter::deserialize_from_json<device_control_messages::error>(json), data_.size() - rest.size());
            else if (msg_type == message_type::measurement)
                return std::make_tuple(json_converter::deserialize_from_json<device_control_messages::measurement>(json), data_.size() - rest.size());
            else
                return std::make_tuple(std::nullopt, 0);
        }
        catch (const nlohmann::json::exception& e_)
        {
            return std::make_tuple(std::nullopt, 0);
        }
    }
};
}

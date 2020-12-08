#pragma once

#include <sstream>
#include <tuple>

#include <nlohmann/json.hpp>

#include <device_control_messages/messages.h>

namespace hw::device_control_messages
{

namespace json_converter
{

/**
 * @brief Serialize device control message into JSON.
 *
 * @tparam MessageType Type of message to serialize
 * @param message_ Message to serialize
 * @return Message in JSON format.
 */
template <class MessageType>
nlohmann::json serialize_to_json(const MessageType& message_)
{
    return nlohmann::json(message_);
}

/**
 * @brief Deserialize device control message into JSON
 *
 * @tparam MessageType Type of message to serialize
 * @param json_ JSON object to deserialize
 * @return Message if the json object can be deserialized as MessageType type, std::nullopt otherwise.
 */
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
constexpr char err_type[]            = "err_type";
}

NLOHMANN_JSON_SERIALIZE_ENUM(device_control_messages::message_type,
                             {
                                 {device_control_messages::message_type::measurement, "measurement"},
                                 {device_control_messages::message_type::error, "error"},
                             })

/**
 * @brief Convert measurement device control message to JSON
 *
 * @param json_ Measurement message serialized to JSON
 * @param measurement_ Input message
 */
inline void to_json(nlohmann::json& json_, const measurement& measurement_)
{
    json_[keys::device_name]         = measurement_.device_name;
    json_[keys::message_type]        = measurement_.msg_type;
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

/**
 * @brief Convert JSON message to measurement device control message
 *
 * @param json_ Input JSON object
 * @param measurement_ Output measurement
 */
inline void from_json(const nlohmann::json& json_, measurement& measurement_)
{
    json_.at(keys::device_name).get_to(measurement_.device_name);
    json_.at(keys::message_type).get_to(measurement_.msg_type);
    json_.at(keys::temperature_sensors).get_to(measurement_.temperature_sensors);
    json_.at(keys::fans_speed).get_to(measurement_.fans_speed);
}

NLOHMANN_JSON_SERIALIZE_ENUM(error::error_type,
                             {
                                 {device_control_messages::error::error_type::disc_corrupted, "disk_corrupted"},
                                 {device_control_messages::error::error_type::exploded, "exploded"},
                                 {device_control_messages::error::error_type::unknown, "unknown"},
                             })

/**
 * @brief Convert error device control message to JSON
 *
 * @param json_ Error message serialized to JSON
 * @param measurement_ Input message
 */
inline void to_json(nlohmann::json& json_, const error& error_)
{
    json_[keys::device_name]  = error_.device_name;
    json_[keys::message_type] = error_.msg_type;
    json_[keys::err_type]     = error_.err_type;
}

/**
 * @brief Convert JSON message to error device control message
 *
 * @param json_ Input JSON object
 * @param measurement_ Output error
 */
inline void from_json(const nlohmann::json& json_, error& error_)
{
    json_.at(keys::device_name).get_to(error_.device_name);
    json_.at(keys::message_type).get_to(error_.msg_type);
    json_.at(keys::err_type).get_to(error_.err_type);
}

/** @brief Serialer/Deserializer for device control messages to JSON (and vice versa) */
class json_serializer
{
public:
    /**
     * @brief Serialize any device control message to byte vector
     *
     * @param message_ Input message
     * @return Input message serialized to bytes
     */
    static std::vector<common::byte_t> serialize(const device_message_type& message_)
    {
        auto serializing_visitor = [](auto msg_) {
            auto str_json = json_converter::serialize_to_json(msg_).dump();
            std::vector<common::byte_t> buffer(str_json.size());
            std::copy(str_json.begin(), str_json.end(), buffer.begin());
            return buffer;
        };

        return std::visit(serializing_visitor, message_);
    }

    /**
     * @brief Deserialize byte vector to device control message
     *
     * @param data_ Data to deserialize from
     * @return Tuple: 1. message, if it can be deserialized from the provied data, 2. number of bytes consumed from the input vector.
     */
    static std::tuple<std::optional<device_message_type>, size_t> deserialize(const std::vector<common::byte_t>& data_)
    {
        try
        {
            // NOTE: In nlohmann::json library, incremental json parsing can be achieved only if input data is provided in std::istream.
            // Therefore, if the application allows sending multiple json messages via one connection, data must be copied into input
            // stream first.
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

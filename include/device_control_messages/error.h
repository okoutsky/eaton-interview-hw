#pragma once

#include <string>

#include <device_control_messages/header.h>
#include <device_control_messages/message_types.h>

namespace hw::device_control_messages
{
/** @brief Message reporting device error */
class error : public header
{
public:
    /** @brief Possible error causes */
    enum class error_type : uint8_t
    {
        disc_corrupted = 0, ///< Disk got corrupted
        exploded       = 1, ///< Part of device exploded
        unknown        = 2, ///< Unknown error cause
    };

public:
    error() = default;

    /**
     * @brief Constructor
     *
     * @param device_name_ Name of device
     * @param error_ Error type
     */
    error(const std::string& device_name_, error_type error_)
        : header(device_name_, message_type::error)
        , err_type(error_)
    {}

    std::string as_string() const override;

public:
    error_type err_type; ///< Error type

private:
    // Serialize error type to string
    static std::string error_type_to_string(error_type error_)
    {
        if (error_ == error_type::disc_corrupted)
            return "disk_corrupted";
        if (error_ == error_type::exploded)
            return "exploded";
        return "unknown";
    }
};

}

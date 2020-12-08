# Solution Description

### System components

1. Devices
    - Abstraction of devices represented by `device_base` interface.
    - Each device is identified by a unique name.
    - Devices report new messages (measurements, errors...) with `device_base::on_mesage` callback.
    - For demonstration purposes `file_reading_device` was implemented. Instances of this class read temperature and fan speed values from provided files and report measurements in regular intervals.
1. Device control messages
    - Definition of messages sent from devices to device monitoring center.
    - Two types of messages are currently implemented: `measurement` and `error`. Each message contains a common header consisting of device name and message type.
    - For demonstration purposes, messages for one-way communication from devices to devices monitor center were implemented only.
    - Component also contains functions for serializing/deserializing device control messages to/from JSON format.
1. Device messages storage
1. Network library for TCP communication between devices and device control center.
    - `device_tcp_connection` - Provides functionality for transmitting device control messages over TCP. Messages are serialized to a transporting format or deserialized back. Serializing/deserializing is independent from `device_tcp_connection` implementation. In the demonstration scenario (description [here](./build-and-run.sh)), messages are serialized to/from JSON format.
    - `device_tcp_server` - Instances of this class listen on provided IP address and TCP port for connections from devices. New messages are signaled by invoking `device_tcp_server::on_message` callback.
    - `device_tcp_client` - Instances connect to `device_tcp_server` using TCP and send device control messages to it.

### Used third party libraries
1. Boost.Asio
    - https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio.html
    - Used in network accessing classes and for running asynchronous timer in `file_reading_device` class.
1. Boost.Program_options
    - https://www.boost.org/doc/libs/1_74_0/doc/html/program_options.html
    - Parsing command line parameters for executable tools.
1. Nlohmann::JSON
    - https://github.com/nlohmann/json
    - Serializing and deserializing device controlling messages to/from JSON.
1. Catch2
    - https://github.com/catchorg/Catch2
    - Unit testing.

Popis reseni:

4. storage zprav poskytujici metody pro zpetne cteni a analyzovani prijatych zprav.

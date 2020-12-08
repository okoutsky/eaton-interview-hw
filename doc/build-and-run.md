# Building and Running

## Dependencies
- CMake version 3.16.3
- Boost.Asio C++ library version 1.74.1 or higher
- Nlohman::JSON C++ library version 3.7.3 or higher.
- **Optional** (for building and running tests only): Catch2 framework version 2.0.0 or higher.

## Docker environment
Requirements for installing all dependencies can be avoided using provided Docker environment image [environment.Dockerfile](../dockerfile/environment.Dockerfile).

Run the following commands from the project root directory (might need sudo).

#### Build and run container
```
docker build --tag okoutsky-hw-env -f ./dockerfile/environment.Dockerfile .
docker run --rm -v$(pwd):/sources -d --name okoutsky-hw-env -it okoutsky-hw-env:latest
```

#### Stop container
```
docker stop okoutsky-hw-env
```

## Building code
#### Getting project source
```
git clone https://github.com/okoutsky/eaton-interview-hw.git
cd eaton-interview-hw
```

#### Build directly
```
cmake -B./build -H.
cmake --build ./build
```

#### Build usinng Docker container
Build and run the container as described in the previous section and then:
```
docker exec -it okoutsky-hw-env cmake -B/build -H/sources -GNinja
docker exec -it okoutsky-hw-env cmake --build /build
```

## Running
### Demo scenario

There are 2 network devices and a device monitoring center acting as a server. The monitoring center will print statistics about received messages in 3s intervals.

#### Running directly
In first terminal run device monitoring center:
```
./build/tools/device_monitor_tool/device_monitor_tool --ip 127.0.0.1 --port 12345 --stats-print-interval 3
```

In second terminal run Device1:
```
./build/tools/file_reading_device_tool/file_reading_device_tool --server-ip 127.0.0.1 --server-port 12345 --device-name Device1 -t /sys/class/hwmon/hwmon4/temp1_input -f /sys/class/hwmon/hwmon2/fan1_input
```

In third terminal run Device2:
```
./build/tools/file_reading_device_tool/file_reading_device_tool --server-ip 127.0.0.1 --server-port 12345 --device-name Device2 -t /sys/class/hwmon/hwmon4/temp2_input -t /sys/class/hwmon/hwmon4/temp3_input
```

#### Running in Docker environment
In first terminal run device monitoring center:
```
docker exec -it okoutsky-hw-env /build/tools/device_monitor_tool/device_monitor_tool --ip 127.0.0.1 --port 12345 --stats-print-interval 3
```

In second terminal run Device1:
```
docker exec -it okoutsky-hw-env /build/tools/file_reading_device_tool/file_reading_device_tool --server-ip 127.0.0.1 --server-port 12345 --device-name Device1 -t /sys/class/hwmon/hwmon4/temp1_input -f /sys/class/hwmon/hwmon2/fan1_input
```

In third terminal run Device2:
```
docker exec -it okoutsky-hw-env /build/tools/file_reading_device_tool/file_reading_device_tool --server-ip 127.0.0.1 --server-port 12345 --device-name Device2 -t /sys/class/hwmon/hwmon4/temp2_input -t /sys/class/hwmon/hwmon4/temp3_input
```

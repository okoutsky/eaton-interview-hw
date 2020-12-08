# Building and Running

## Dependencies
- CMake version 3.16.3
- Boost.Asio C++ library version 1.74.1 or higher
- Nlohman::JSON C++ library version 3.7.3 or higher.
- **Optional** (for building and running tests only): Catch2 framework version 2.0.0 or higher.

## Docker environment
Requirements for installing all dependencies can be avoided using provided Docker environment image [environment.Dockerfile](../dockerfile/environment.Dockerfile).

### Building and running Docker environment image
Run from the project root directory (might need sudo).

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
Getting project source:

```
git clone https://github.com/okoutsky/eaton-interview-hw.git
cd eaton-interview-hw
```

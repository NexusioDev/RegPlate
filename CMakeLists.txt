cmake_minimum_required(VERSION 4.1)
project(RegPlate)

set(CMAKE_CXX_STANDARD 14)

include_directories(nlohmann)

add_executable(RegPlate
        nlohmann/json.hpp
        main.cpp)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/kfz.json
        ${CMAKE_CURRENT_BINARY_DIR}/kfz.json
        COPYONLY
)
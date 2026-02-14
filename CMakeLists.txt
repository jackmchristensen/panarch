cmake_minimum_required(VERSION 3.16)
project(Panarch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(Panarch
  src/main.cpp
)

qt_add_resources(Panarch "qml"
  PREFIX "/"
  FILES
    src/qml/main.qml
)

target_include_directories(Panarch PRIVATE src)
target_link_libraries(Panarch PRIVATE Qt6::Quick)

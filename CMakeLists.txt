cmake_minimum_required(VERSION 3.16)
project(Panarch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(Panarch
  src/main.cpp
  src/backend/AssetIndex.cpp
  src/backend/AssetListModel.cpp
  src/backend/Backend.cpp
)

qt_add_resources(Panarch "ui"
  PREFIX "/"
  FILES
    src/ui/qmldir
    src/ui/Theme.qml
    src/ui/PButton.qml
    src/ui/Main.qml
)

target_include_directories(Panarch PRIVATE src)
target_link_libraries(Panarch PRIVATE Qt6::Quick)

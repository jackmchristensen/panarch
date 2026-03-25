cmake_minimum_required(VERSION 3.16)
project(Panarch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(X11 REQUIRED)
find_package(pxr REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Quick Gui OpenGL)

qt_standard_project_setup()

qt_add_executable(thumbnail_generator
  src/processes/ThumbnailGeneratorMain.cpp
  src/processes/ThumbnailGenerator.cpp
)
target_include_directories(thumbnail_generator PRIVATE
  src
  ${PXR_INCLUDE_DIRS}
  /usr/include/python3.14
)
target_link_libraries(thumbnail_generator PRIVATE
  Qt6::OpenGL
  Qt6::Gui
  usd_m
)

qt_add_executable(usd_inspector
  src/processes/UsdInspectorMain.cpp
  src/processes/UsdInspector.cpp
)
target_include_directories(usd_inspector PRIVATE
  src
  ${PXR_INCLUDE_DIRS}
  /usr/include/python3.14
)
target_link_libraries(usd_inspector PRIVATE
  Qt6::Gui
  usd_m
)

qt_add_executable(scan_assets
  src/processes/ScanAssetsMain.cpp
  src/processes/ScanAssets.cpp
)
target_include_directories(scan_assets PRIVATE
  src
  ${PXR_INCLUDE_DIRS}
  /usr/include/python3.14
)
target_link_libraries(scan_assets PRIVATE
  Qt6::Gui
  usd_m
)

qt_add_executable(Panarch
  src/main.cpp
  src/backend/AssetListModel.cpp
  src/backend/AssetFilterModel.cpp
  src/backend/Backend.cpp
)

qt_add_resources(Panarch "ui"
  PREFIX "/"
  FILES
    src/ui/qmldir
    src/ui/Theme.qml
    src/ui/PButton.qml
    src/ui/PMenuButton.qml
    src/ui/PIndicator.qml
    src/ui/PTooltip.qml
    src/ui/PInfoRow.qml
    src/ui/PInfoList.qml
    src/ui/PSectionHeader.qml
    src/ui/Main.qml
)

file(GLOB HDRI_FILES "${CMAKE_SOURCE_DIR}/src/resources/hdri/*.exr")

add_custom_command(TARGET thumbnail_generator POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "$<TARGET_FILE_DIR:thumbnail_generator>/resources/hdri"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${HDRI_FILES}
          "$<TARGET_FILE_DIR:thumbnail_generator>/resources/hdri"
  COMMENT "Copying HDRI resources"
)

target_include_directories(Panarch PRIVATE
  src
)

target_link_libraries(Panarch PRIVATE
  Qt6::Quick
)

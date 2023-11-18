cmake_minimum_required(VERSION 3.5)

set(OWN_COMPONENTS_DIR "${CMAKE_SOURCE_DIR}/src")
set(RENAMED_MAIN_DIR "${CMAKE_SOURCE_DIR}/src/esp32")

set(EXTRA_COMPONENT_DIRS ${OWN_COMPONENTS_DIR} ${RENAMED_MAIN_DIR}) # for the main file in ESP32

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Deep_Bird)
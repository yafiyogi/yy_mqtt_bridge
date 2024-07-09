#
#
#  MIT License
#
#  Copyright (c) 2024 Yafiyogi
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#
#

# cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE .
# PKG_CONFIG_PATH=${HOME}/code/tp/lib/pkgconfig:${HOME}/code/tp/share/pkgconfig:${PKG_CONFIG_PATH} cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE .

cmake_minimum_required(VERSION 3.24)

project(yy_mqtt_bridge LANGUAGES CXX)

set(YY_ROOT "${PROJECT_SOURCE_DIR}/..")
set(YY_CMAKE "${YY_ROOT}/yy_cmake")

include(${YY_CMAKE}/cmake_common.txt)
include(${YY_CMAKE}/cmake_boost-1.84.0.txt)

find_package(Boost 1.84.0 REQUIRED COMPONENTS json locale)
find_package(fmt REQUIRED)
find_package(re2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_check_modules(Mosquitto_c IMPORTED_TARGET libmosquitto_static REQUIRED)
pkg_check_modules(Mosquitto_cpp IMPORTED_TARGET libmosquittopp_static REQUIRED)
pkg_check_modules(LibCrypto IMPORTED_TARGET libcrypto REQUIRED)
pkg_check_modules(OpenSSL IMPORTED_TARGET libssl REQUIRED)
pkg_check_modules(LibSSL IMPORTED_TARGET libssl REQUIRED)
pkg_check_modules(ZLib IMPORTED_TARGET zlib REQUIRED)

add_executable(yy_mqtt_bridge
  configure_mqtt.cpp
  configure_mqtt_handlers.cpp
  configure_mqtt_topics.cpp
  configure_prometheus.cpp
  configure_prometheus_label_actions.cpp
  configure_prometheus_metrics.cpp
  prometheus_replace_path_label_action.cpp
  prometheus_labels.cpp
  prometheus_metric.cpp
  prometheus_style.cpp
  logger.cpp
  mqtt_client.cpp
  mqtt_handler.cpp
  mqtt_json_handler.cpp
  mqtt_value_handler.cpp
  yy_mqtt_bridge.cpp )

target_compile_options(yy_mqtt_bridge
  PRIVATE
  "-DSPDLOG_COMPILED_LIB"
  "-DSPDLOG_FMT_EXTERNAL")

target_include_directories(yy_mqtt_bridge
  PRIVATE
    "${CMAKE_INSTALL_PREFIX}/include"
    "${YY_THIRD_PARTY_LIBRARY}" )

target_link_directories(yy_mqtt_bridge
  PUBLIC
  "${CMAKE_INSTALL_PREFIX}/lib"
  /opt/local/lib )

target_link_libraries(yy_mqtt_bridge
  yy_json
  yy_mqtt
  yy_cpp
  Boost::json
  Boost::locale
  fmt::fmt
  re2::re2
  spdlog::spdlog
  yaml-cpp::yaml-cpp
  PkgConfig::Mosquitto_cpp
  PkgConfig::Mosquitto_c
  PkgConfig::LibCrypto
  PkgConfig::LibSSL
  PkgConfig::ZLib )

install(TARGETS yy_mqtt_bridge )

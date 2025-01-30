## Dependencies:
- yy_cmake
- yy_cpp
- yy_json
- yy_mqtt
- yy_prometheus
- yy_web

## 3rd Party Dependencies
- Boost
  - Json
  - locale
  - program_options
- civetweb-cpp
- civetweb
- fmt
- re2
- spdlog
- yaml-cpp
- Mosquitto_cpp
- Mosquitto_c
- LibreSSL Crypto
- LibreSSL SSL
- Zlib

## Build Everything
These commands are run in the source directory.
### Initial Build All
<code>./build_type_all Release</code>
### Re-build All
<code>./build all</code>
### Clean All
<code>./build clean</code>

## Build mqtt_bridge
These commands are run in the build-dir directory
### Initial Build
<code>cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/<your path here> -DYY_THIRD_PARTY_LIBRARY=${HOME}/<third party includes path> -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release ..</code>
### Rebuild
<code>make all</code>
### Clean
<code>make clean</code>

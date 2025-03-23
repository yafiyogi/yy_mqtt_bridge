## Dependencies:
- yy_cmake
- yy_cpp
- yy_json
- yy_mqtt
- yy_prometheus
- yy_web

## 3rd Party Dependencies
See [yafiyogi/Third-Party-Library-Build-Instructions.txt](https://gist.github.com/yafiyogi/8cc25cca91b6b38f50906ca8d5667bf2) for build instructions.
- Boost
  - Json
  - locale requires:
    - libiconv
    - libicudata
    - libicui18n
    - libicuuc
  - program_options
- civetweb
  - civetweb-cpp
  - civetweb
- fmt
- re2 requires:
  - abseil-cpp
- spdlog
- yaml-cpp
- Mosquitto
  - Mosquitto_cpp
  - Mosquitto_c
- LibreSSL (openssl might also work. I haven't tried.)
  - libcrypto
  - libssl
- Zlib

## Build Everything
These commands are run in the source directory.
### Initial Build All
<code>./build_type_all Release</code>
### Re-build All
<code>./build all</code>
### Clean All
<code>./build clean</code>

## Build mqtt_bridge only
These commands are run in the build-dir directory
### Initial Build
<code>cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/\<your path here\> -DYY_THIRD_PARTY_LIBRARY=${HOME}/\<third party includes path\> -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release ..</code>
### Rebuild
<code>make all</code>
### Clean
<code>make clean</code>

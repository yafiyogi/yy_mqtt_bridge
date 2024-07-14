/*

  MIT License

  Copyright (c) 2024 Yafiyogi

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <unistd.h>
#include <csignal>

#include <memory>

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"
#include "mosquittopp.h"

#include "yy_cpp/yy_locale.h"

#include "configure_mqtt.h"
#include "configure_prometheus.h"
#include "logger.h"
#include "mqtt_client.h"
#include "mqtt_handler.h"

namespace {

static std::atomic<bool> exit_program{false};

void signal_handler(int /* signal */)
{
  std::signal(SIGINT, SIG_DFL);
  std::signal(SIGTERM, SIG_DFL);
  exit_program = true;
}

}

int main()
{
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  mosqpp::lib_init();

  yafiyogi::mqtt_bridge::set_console_logger();
  spdlog::set_level(spdlog::level::debug);
  yafiyogi::yy_locale::set_locale();

  const YAML::Node yaml_config = YAML::LoadFile("mqtt_bridge.yaml");


  const auto yaml_prometheus = yaml_config["prometheus"];

  if(!yaml_prometheus)
  {
    spdlog::error("Not found prometheus config");
    return 1;
  }

  auto prometheus_config = yafiyogi::mqtt_bridge::prometheus::configure_prometheus(yaml_prometheus);

  const auto yaml_mqtt = yaml_config["mqtt"];

  if(!yaml_mqtt)
  {
    spdlog::error("Not found mqtt config");
    return 1;
  }

  auto mqtt_config{yafiyogi::mqtt_bridge::configure_mqtt(yaml_mqtt, prometheus_config)};

  auto client = std::make_unique<yafiyogi::mqtt_bridge::mqtt_client>(mqtt_config);

  client->connect();
  while(!exit_program)
  {
    if(auto rc = client->loop();
       rc)
    {
      spdlog::info("reconnect [{}]", rc);
      client->reconnect();
    }
  }

  client->disconnect();

  while(client->is_connected())
  {
    sleep(1);
  }

  mosqpp::lib_cleanup();

  spdlog::info("Ende");
  yafiyogi::mqtt_bridge::stop_all_logs();
  return 0;
}

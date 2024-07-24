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

#include <exception>
#include <memory>

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"
#include "mosquittopp.h"

#include "yy_cpp/yy_locale.h"

#include "yy_prometheus/yy_prometheus_cache.h"

#include "yy_web/yy_web_server.h"

#include "configure_mqtt.h"
#include "configure_prometheus.h"
#include "logger.h"
#include "mqtt_client.h"
#include "mqtt_handler.h"
#include "prometheus_civetweb_handler.h"

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
  using namespace yafiyogi;
  using namespace std::string_view_literals;


  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  mqtt_bridge::set_console_logger();
  spdlog::set_level(spdlog::level::debug);
  yy_locale::set_locale();

  const YAML::Node yaml_config = YAML::LoadFile("mqtt_bridge.yaml");

  const auto yaml_prometheus = yaml_config["prometheus"sv];
  if(!yaml_prometheus)
  {
    spdlog::error("Not found prometheus config"sv);

    return 1;
  }

  auto prometheus_config{mqtt_bridge::prometheus::configure_prometheus(yaml_prometheus)};

  const auto yaml_mqtt = yaml_config["mqtt"sv];
  if(!yaml_mqtt)
  {
    spdlog::error("Not found mqtt config"sv);

    return 1;
  }

  auto metric_cache = std::make_shared<yy_prometheus::MetricDataCache>();

  auto http_server{std::make_unique<yy_web::WebServer>(prometheus_config.options)};
  http_server->AddHandler(prometheus_config.uri,
                          std::make_unique<mqtt_bridge::prometheus::PrometheusWebHandler>(metric_cache));

  mosqpp::lib_init();

  auto mqtt_config{mqtt_bridge::configure_mqtt(yaml_mqtt,
                                               prometheus_config)};

  auto client = std::make_unique<mqtt_bridge::mqtt_client>(mqtt_config,
                                                           metric_cache);

  client->connect();
  try
  {
    while(!exit_program)
    {
      if(auto rc = client->loop();
         rc)
      {
        spdlog::info("reconnect [{}]"sv, rc);
        client->reconnect();
      }
    }
  }
  catch(const std::exception & ex)
  {
    spdlog::critical("Exception caught [{}]", ex.what());
  }
  catch(...)
  {
    spdlog::critical("Exception caught!");
  }

  client->disconnect();

  while(client->is_connected())
  {
    sleep(1);
  }

  mosqpp::lib_cleanup();
  http_server.reset();


  spdlog::info("Ende"sv);
  mqtt_bridge::stop_all_logs();
  return 0;
}

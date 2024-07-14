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

#include <string>

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "configure_mqtt_handlers.h"
#include "configure_mqtt_topics.h"
#include "mqtt_handler.h"
#include "prometheus_config.h"
#include "yaml_util.h"

#include "configure_mqtt.h"

namespace yafiyogi::mqtt_bridge {

mqtt_config configure_mqtt(const YAML::Node & yaml_mqtt,
                           prometheus::config & p_prometheus_config)
{
  const auto yaml_host = yaml_mqtt["host"];
  if(!yaml_host)
  {
    spdlog::error("Not found mqtt host\n");
    return mqtt_config{};
  }
  auto host = yaml_host.as<std::string>();

  int port = yaml_get_value(yaml_mqtt["port"], yy_mqtt::mqtt_default_port);

  spdlog::info("host=[{}] port=[{}]\n", host, port);

  auto handlers = configure_mqtt_handlers(yaml_mqtt["handlers"], p_prometheus_config);
  auto [subscriptions, topics] = configure_mqtt_topics(yaml_mqtt["topics"], handlers);

  return mqtt_config{"", std::move(host), port, std::move(handlers), std::move(subscriptions), std::move(topics)};
}

} // namespace yafiyogi::mqtt_bridge

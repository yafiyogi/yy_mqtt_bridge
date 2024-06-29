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

#pragma once

#include "yy_mqtt/yy_mqtt_constants.h"

#include "yaml_small.h"

#include "mqtt_topics.h"

namespace yafiyogi::mqtt_bridge {
namespace prometheus_detail {

struct prometheus_config;

} // namespace prometheus_detail

struct mqtt_config final
{
    std::string id;
    std::string host;
    int port = yy_mqtt::mqtt_default_port;
    MqttHandlerStore handlers;
    Subscriptions subscriptions;
    Topics topics;
};

mqtt_config configure_mqtt(const YAML::Node & yaml_mqtt,
                           prometheus_detail::prometheus_config & prometheus_config);

} // namespace yafiyogi::mqtt_bridge

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

#include "boost/json/basic_parser_impl.hpp"

#include "yy_mqtt/yy_mqtt_types.h"

#include "mqtt_handler.h"
#include "prometheus_metric_query.h"

namespace yafiyogi::mqtt_bridge {

class JsonVisitor;

class MqttJsonHandler final:
      public MqttHandler
{
  public:
    using MetricsJsonPointer = prometheus_detail::MetricsJsonPointer;
    using JsonParserConfig = MetricsJsonPointer::pointers_config_type;
    using JsonParser = boost::json::basic_parser<MetricsJsonPointer>;
    using JsonParserOptions = boost::json::parse_options;

    MqttJsonHandler(std::string_view p_handler_id,
                    const JsonParserOptions & p_json_options,
                    JsonParserConfig && p_json_handler_config) noexcept;

    MqttJsonHandler() = delete;
    MqttJsonHandler(const MqttJsonHandler &) = delete;
    MqttJsonHandler(MqttJsonHandler &&) noexcept = default;
    ~MqttJsonHandler() noexcept override = default;

    MqttJsonHandler & operator=(const MqttJsonHandler &) = delete;
    MqttJsonHandler & operator=(MqttJsonHandler &&) noexcept = default;

    void Event(std::string_view topic,
               const yy_mqtt::TopicLevels & topic_levels,
               std::string_view data) noexcept override;

  private:
    JsonParser m_parser;
    JsonVisitor * m_json_visitor = nullptr;
};

} // namespace yafiyogi::mqtt_bridge

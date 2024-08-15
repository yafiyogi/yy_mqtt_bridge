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

#include <string_view>

#include "yy_prometheus/yy_prometheus_metric_data.h"

#include "mqtt_handler.h"
#include "prometheus_metric.h"
#include "value_type.h"

namespace yafiyogi::mqtt_bridge {

class MqttValueHandler:
      public MqttHandler
{
  public:
    explicit MqttValueHandler(std::string_view p_handler_id,
                              prometheus::Metrics && p_metrics) noexcept;
    constexpr MqttValueHandler() noexcept = default;
    MqttValueHandler(const MqttValueHandler &) = delete;
    constexpr MqttValueHandler(MqttValueHandler &&) noexcept = default;
    constexpr ~MqttValueHandler() noexcept override = default;

    MqttValueHandler & operator=(const MqttValueHandler &) = delete;
    constexpr MqttValueHandler & operator=(MqttValueHandler &&) noexcept = default;

    yy_prometheus::MetricDataVector & Event(std::string_view p_value,
                                            const yy_prometheus::Labels & p_labels,
                                            const int64_t p_timestamp) noexcept override;
  private:
    prometheus::Metrics m_metrics{};
    yy_prometheus::MetricDataVector m_metric_data{};
};

} // namespace yafiyogi::mqtt_bridge

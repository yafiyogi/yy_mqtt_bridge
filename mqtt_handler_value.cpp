/*

  MIT License

  Copyright (c) 2024-2025 Yafiyogi

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

#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_prometheus/yy_prometheus_metric_data.h"

#include "prometheus_metric.h"

#include "mqtt_handler_value.h"

namespace yafiyogi::mqtt_bridge {

using namespace std::string_view_literals;

MqttValueHandler::MqttValueHandler(std::string_view p_handler_id,
                                   prometheus::Metrics && p_metrics) noexcept:
  MqttHandler(p_handler_id, type::Value),
  m_metrics(std::move(p_metrics))
{
  m_metric_data.reserve(m_metrics.size());
}

yy_prometheus::MetricDataVector & MqttValueHandler::Event(std::string_view p_value,
                                                          const yy_prometheus::Labels & p_labels,
                                                          const yy_mqtt::TopicLevelsView & p_levels,
                                                          const int64_t p_timestamp) noexcept
{
  spdlog::debug("  handler [{}]"sv, Id());

  m_metric_data.clear(yy_data::ClearAction::Keep);

  for(auto & metric : m_metrics)
  {
    metric->Event(p_value, p_labels, p_levels, m_metric_data, p_timestamp, ValueType::Unknown);
  }

  return m_metric_data;
}

} // namespace yafiyogi::mqtt_bridge

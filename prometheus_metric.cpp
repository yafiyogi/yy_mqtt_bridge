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
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_prometheus/yy_prometheus_labels.h"
#include "prometheus_metric.h"
#include "prometheus_label_action.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

Metric::Metric(std::string_view p_id,
               const MetricType p_type,
               const MetricUnit p_unit,
               std::string && p_property,
               LabelActions && p_actions):
  m_property(std::move(p_property)),
  m_metric_data(p_id, Labels{}, ""sv, p_type, p_unit),
  m_actions(std::move(p_actions))
{
}

const std::string & Metric::Id() const noexcept
{
  return m_metric_data.Id();
}


Metric::MetricType Metric::Type() const noexcept
{
  return m_metric_data.Type();
}

const std::string & Metric::Property() const noexcept
{
  return m_property;
}

void Metric::Event(std::string_view p_value,
                   const Labels & p_labels,
                   MetricDataVector & p_metric_data,
                   const int64_t p_timestamp)
{
  spdlog::debug("    [{}] property=[{}] [{}]"sv,
                Id(),
                m_property,
                p_value);

  m_metric_data.Value(p_value);
  m_metric_data.Labels(p_labels);
  m_metric_data.Timestamp(p_timestamp);

  for(const auto & action : m_actions)
  {
    action->Apply(p_labels, m_metric_data.Labels());
  }

  m_metric_data.Labels().visit([](const auto & label,
                                  const auto & value) {
    spdlog::debug("      - [{}]:[{}]"sv, label, value);
  });

  p_metric_data.emplace_back(m_metric_data);
}

} // namespace yafiyogi::mqtt_bridge::prometheus

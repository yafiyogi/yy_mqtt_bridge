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

#include <string>
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_values/yy_label_action.hpp"
#include "yy_values/yy_values_labels.hpp"

#include "prometheus_metric.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

Metric::Metric(std::string_view p_id,
               const MetricType p_type,
               const MetricUnit p_unit,
               const MetricTimestamp p_timestamp,
               std::string && p_property,
               yy_values::LabelActions && p_label_actions,
               yy_values::ValueActions && p_value_actions):
  m_property(std::move(p_property)),
  m_metric_data(yy_values::MetricId{p_id}, yy_values::Labels{}, ""sv, p_type, p_unit, p_timestamp),
  m_label_actions(std::move(p_label_actions)),
  m_value_actions(std::move(p_value_actions))
{
}

const std::string & Metric::Id() const noexcept
{
  return m_metric_data.Id().Name();
}


Metric::MetricType Metric::Type() const noexcept
{
  return m_metric_data.MetricType();
}

const std::string & Metric::Property() const noexcept
{
  return m_property;
}

void Metric::Event(std::string_view p_value,
                   const yy_values::Labels & p_labels,
                   const yy_mqtt::TopicLevelsView & p_levels,
                   MetricDataVector & p_metric_data,
                   const timestamp_type p_timestamp,
                   yy_values::ValueType p_value_type)
{
  spdlog::debug("    [{}] property=[{}] [{}]"sv,
                Id(),
                m_property,
                p_value);

  m_metric_data.Value(p_value);
  m_metric_data.Timestamp(p_timestamp);

  auto & l_labels = m_metric_data.Labels();
  l_labels.clear(yy_data::ClearAction::Keep);

  for(const auto & action : m_label_actions)
  {
    action->Apply(p_labels, p_levels, l_labels);
  }

  for(const auto & action : m_value_actions)
  {
    action->Apply(m_metric_data, p_value_type);
  }

  if(spdlog::level::debug == spdlog::get_level())
  {
    m_metric_data.Labels().visit([](const auto & label,
                                    const auto & value) {
      spdlog::debug("      - [{}]:[{}]"sv, label, value);
    });
  }

  p_metric_data.emplace_back(m_metric_data);
}

} // namespace yafiyogi::mqtt_bridge::prometheus

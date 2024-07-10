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

#include "spdlog/spdlog.h"

#include "prometheus_labels.h"
#include "prometheus_metric.h"
#include "prometheus_label_action.h"

namespace yafiyogi::mqtt_bridge::prometheus {

Metric::Metric(std::string_view p_id,
               const MetricType p_type,
               std::string && p_property,
               LabelActions && p_actions):
  m_id(p_id),
  m_type(p_type),
  m_property(std::move(p_property)),
  m_actions(std::move(p_actions))
{
}

const std::string & Metric::Id() const noexcept
{
  return m_id;
}


MetricType Metric::Type() const noexcept
{
  return m_type;
}

const std::string & Metric::Property() const noexcept
{
  return m_property;
}

void Metric::Event(std::string_view p_data,
                   const prometheus::Labels & p_labels)
{
  spdlog::debug("    [{}] property=[{}] [{}]",
                m_id,
                m_property,
                p_data);
  for(const auto & action : m_actions)
  {
    auto metric_labels{p_labels};

    action->Apply(p_labels, metric_labels);

    metric_labels.visit([](const auto & label,
                           const auto & value) {
      spdlog::debug("      - [{}]:[{}]", label, value);
    });
  }
}

} // namespace yafiyogi::mqtt_bridge::prometheus

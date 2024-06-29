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

#include "configure_prometheus_config.h"

namespace yafiyogi::mqtt_bridge::prometheus_detail {

Metric::Metric(std::string_view p_id,
               const MetricType p_type,
               std::string && p_property):
  m_id(p_id),
  m_type(p_type),
  m_property(std::move(p_property))
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

void Metric::Event(const Labels & p_labels,
                   std::string_view p_data)
{
  spdlog::debug("\t\t[{}] property=[{}] labels=[{}] topic=[{}] data=[{}]",
                m_id,
                m_property,
                p_labels.size(),
                p_labels.get_label("topic"),
                p_data);
}

} // namespace yafiyogi::mqtt_bridge::prometheus_detail

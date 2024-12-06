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

#include <memory>
#include <string>
#include <string_view>

#include "yy_cpp/yy_vector.h"
#include "yy_cpp/yy_flat_map.h"

#include "yy_mqtt/yy_mqtt_types.h"

#include "yy_prometheus/yy_prometheus_metric_data.h"

#include "label_action.h"
#include "value_action.h"
#include "value_type.h"

namespace yafiyogi::mqtt_bridge::prometheus {

class Metric final
{
  public:
    using MetricType = yy_prometheus::MetricType;
    using MetricUnit = yy_prometheus::MetricUnit;
    using MetricData = yy_prometheus::MetricData;
    using MetricTimestamp = yy_prometheus::MetricTimestamp;
    using MetricDataVector = yy_prometheus::MetricDataVector;

    explicit Metric(std::string_view p_id,
                    const Metric::MetricType p_type,
                    const MetricUnit p_unit,
                    const MetricTimestamp p_timestamp,
                    std::string && p_property,
                    LabelActions && p_label_actions,
                    ValueActions && p_value_actions);

    constexpr Metric() noexcept = default;
    constexpr Metric(const Metric &) noexcept = default;
    constexpr Metric(Metric &&) noexcept = default;

    Metric & operator=(const Metric &) noexcept = default;
    Metric & operator=(Metric &&) noexcept = default;

    [[nodiscard]]
    const std::string & Id() const noexcept;

    [[nodiscard]]
    Metric::MetricType Type() const noexcept;

    [[nodiscard]]
    const std::string & Property() const noexcept;

    void Event(std::string_view p_value,
               const yy_prometheus::Labels & p_labels,
               MetricDataVector & p_metric_data,
               const int64_t p_timestamp,
               ValueType p_value_type);

  private:
    std::string m_property{};
    MetricData m_metric_data{};
    LabelActions m_label_actions{};
    ValueActions m_value_actions{};
};

using MetricPtr = std::shared_ptr<Metric>;
using Metrics = yy_quad::simple_vector<MetricPtr>;
using MetricsMap = yy_data::flat_map<std::string, Metrics>;

} // namespace yafiyogi::mqtt_bridge::prometheus

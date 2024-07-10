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

#include "prometheus_label_action.h"

namespace yafiyogi::mqtt_bridge::prometheus {

class Labels;

enum class MetricType {Guage};

class Metric final
{
  public:
    explicit Metric(std::string_view p_id,
                    const MetricType p_type,
                    std::string && p_property,
                    LabelActions && p_actions);

    constexpr Metric() noexcept = default;
    constexpr Metric(const Metric &) noexcept = default;
    constexpr Metric(Metric &&) noexcept = default;
    constexpr ~Metric() noexcept = default;

    constexpr Metric & operator=(const Metric &) noexcept = default;
    constexpr Metric & operator=(Metric &&) noexcept = default;

    [[nodiscard]]
    const std::string & Id() const noexcept;

    [[nodiscard]]
    MetricType Type() const noexcept;

    [[nodiscard]]
    const std::string & Property() const noexcept;

    void Event(std::string_view p_data,
               const Labels & p_labels);

  private:
    std::string m_id;
    MetricType m_type;
    std::string m_property;
    LabelActions m_actions;
};

using MetricPtr = std::shared_ptr<Metric>;
using Metrics = yy_quad::simple_vector<MetricPtr>;
using MetricsMap = yy_data::flat_map<std::string, Metrics>;

} // namespace yafiyogi::mqtt_bridge::prometheus

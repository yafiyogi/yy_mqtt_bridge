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

namespace yafiyogi::mqtt_bridge::prometheus_detail {

enum class MetricType {Guage};

class Labels final
{
  public:
    Labels() noexcept = default;
    Labels(const Labels &) noexcept = default;
    Labels(Labels &&) noexcept = default;
    ~Labels() noexcept = default;

    Labels & operator=(const Labels &) noexcept = default;
    Labels & operator=(Labels &&) noexcept = default;

    void clear() noexcept;
    void set_label(std::string_view p_label,
                   std::string p_value);
    void set_label(std::string_view p_label,
                   std::string_view p_value)
    {
      set_label(p_label, std::string{p_value});
    }
    void set_label(std::string_view p_label,
                   yy_mqtt::TopicLevels p_path);

    [[nodiscard]]
    const std::string & get_label(const std::string & p_label) const;

    [[nodiscard]]
    size_t size() const noexcept
    {
      return m_labels.size();
    }

  private:
    using LabelStore = yy_data::flat_map<std::string, std::string>;
    LabelStore m_labels;
    yy_mqtt::TopicLevels m_path;
};

class Metric
{
  public:
    explicit Metric(std::string_view p_id,
                    const MetricType p_type,
                    std::string && p_property);
    [[nodiscard]]
    const std::string & Id() const noexcept;
    [[nodiscard]]
    MetricType Type() const noexcept;
    [[nodiscard]]
    const std::string & Property() const noexcept;

    void Event(const Labels & p_labels,
               std::string_view p_data);

  private:
    std::string m_id;
    MetricType m_type;
    std::string m_property;
};

using MetricPtr = std::shared_ptr<Metric>;
using Metrics = yy_quad::simple_vector<MetricPtr>;
using MetricsMap = yy_data::flat_map<std::string, Metrics>;

inline constexpr int prometheus_default_port = 9100;

struct prometheus_config final
{
    int port = prometheus_default_port;
    MetricsMap metrics{};
};

} // namespace yafiyogi::mqtt_bridge::prometheus_detail

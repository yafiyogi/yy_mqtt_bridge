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

#pragma once

#include <string_view>

#include "yy_cpp/yy_types.hpp"
#include "yy_mqtt/yy_mqtt_types.h"

#include "yy_values/yy_values_metric_data.hpp"

#include "mqtt_handler_fwd.h"
#include "prometheus_metric.h"

namespace yafiyogi::mqtt_bridge {

class MqttHandler
{
  public:
    enum class type:uint8_t {Json, Text, Value};

    explicit MqttHandler(std::string_view p_handler_id,
                         const type p_type,
                         size_type p_metric_count) noexcept;
    constexpr MqttHandler() noexcept = default;
    MqttHandler(const MqttHandler &) = delete;
    MqttHandler(MqttHandler && p_other) noexcept;

    constexpr virtual ~MqttHandler() noexcept = default;

    MqttHandler & operator=(const MqttHandler &) = delete;
    MqttHandler & operator=(MqttHandler && p_other) noexcept;

    [[nodiscard]]
    constexpr const std::string & Id() const noexcept
    {
      return m_handler_id;
    }

    [[nodiscard]]
    constexpr type Type() const noexcept
    {
      return m_type;
    }

    [[nodiscard]]
    constexpr size_type MetricCount() const noexcept
    {
      return m_metric_count;
    }

    virtual void Event(std::string_view p_mqtt_data,
                       const std::string_view p_topic,
                       const yy_mqtt::TopicLevelsView & p_levels ,
                       const timestamp_type p_timestamp,
                       yy_prometheus::MetricDataVectorPtr p_metric_data) noexcept = 0;

  private:
    size_type m_metric_count = 0;
    std::string m_handler_id{};
    type m_type = type::Text;
};

} // namespace yafiyogi::mqtt_bridge

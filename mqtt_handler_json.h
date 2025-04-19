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

#include <cstdint>

#include "boost/json/basic_parser_impl.hpp"

#include "yy_json/yy_json_pointer.h"
#include "yy_mqtt/yy_mqtt_types.h"

#include "yy_prometheus/yy_prometheus_metric_data.h"

#include "mqtt_handler.h"
#include "prometheus_metric.h"
#include "value_type.h"

namespace yafiyogi::mqtt_bridge {
namespace json_handler_detail {

class JsonVisitor final
{
  public:
    using MetricDataVector = yy_prometheus::MetricDataVector;
    using Metrics = prometheus::Metrics;

    constexpr JsonVisitor() noexcept = default;
    constexpr JsonVisitor(const JsonVisitor &) noexcept = default;
    JsonVisitor(JsonVisitor && p_other) noexcept;

    constexpr JsonVisitor & operator=(const JsonVisitor &) noexcept = default;
    JsonVisitor & operator=(JsonVisitor && p_other) noexcept;

    void levels(const yy_mqtt::TopicLevelsView * p_levels) noexcept;
    void metric_data(yy_prometheus::MetricDataVectorPtr p_metric_data) noexcept;
    void topic(const std::string_view p_topic) noexcept;
    void timestamp(const timestamp_type p_timestamp) noexcept;
    void reset() noexcept;

    void apply_str(Metrics & metrics,
                   std::string_view str)
    {
      apply(metrics, str, yy_values::ValueType::String);
    }

    void apply_int64(Metrics & metrics,
                     std::string_view raw,
                     std::int64_t /* num */)
    {
      apply(metrics, raw, yy_values::ValueType::Int);
    }

    void apply_uint64(Metrics & metrics,
                      std::string_view raw,
                      std::uint64_t /* num */)
    {
      apply(metrics, raw, yy_values::ValueType::UInt);
    }

    void apply_double(Metrics & metrics,
                      std::string_view raw,
                      double /* num */)
    {
      apply(metrics, raw, yy_values::ValueType::Float);
    }

    void apply_bool(Metrics & metrics,
                    bool flag)
    {
      apply(metrics, flag ? g_true_str : g_false_str, yy_values::ValueType::Bool);
    }

  private:
    void apply(Metrics & p_metrics,
               std::string_view p_data,
               yy_values::ValueType p_value_type);

    static constexpr const std::string_view g_true_str{"true"};
    static constexpr const std::string_view g_false_str{"false"};
    static const yy_mqtt::TopicLevelsView g_empty_levels;

    yy_data::observer_ptr<std::add_const_t<yy_mqtt::TopicLevelsView>> m_levels{&g_empty_levels};
    yy_prometheus::MetricDataVectorPtr m_metric_data{};
    timestamp_type m_timestamp{};
    std::string_view m_topic{};
};

} // namespace json_handler_detail

class MqttJsonHandler final:
      public MqttHandler
{
  public:
    using builder_type = yy_json::json_pointer_builder<prometheus::Metrics, json_handler_detail::JsonVisitor>;
    using handler_type = builder_type::handler_type;
    using handler_config_type = handler_type::pointers_config_type;
    using parser_type = boost::json::basic_parser<handler_type>;
    using parser_options_type = boost::json::parse_options;

    explicit MqttJsonHandler(std::string_view p_handler_id,
                             const parser_options_type & p_json_options,
                             handler_config_type && p_json_handler_config,
                             size_type p_metric_count) noexcept;

    MqttJsonHandler() = delete;
    MqttJsonHandler(const MqttJsonHandler &) = delete;
    constexpr MqttJsonHandler(MqttJsonHandler &&) noexcept = default;

    MqttJsonHandler & operator=(const MqttJsonHandler &) = delete;
    constexpr MqttJsonHandler & operator=(MqttJsonHandler &&) noexcept = default;

    void Event(std::string_view p_mqtt_data,
               const std::string_view p_topic,
               const yy_mqtt::TopicLevelsView & p_levels ,
               const timestamp_type p_timestamp,
               yy_prometheus::MetricDataVectorPtr p_metric_data) noexcept override;

  private:
    parser_type m_parser;
};

} // namespace yafiyogi::mqtt_bridge

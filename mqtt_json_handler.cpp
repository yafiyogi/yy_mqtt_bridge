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

#include <memory>

#include "boost/json/basic_parser_impl.hpp"
#include "spdlog/spdlog.h"

#include "prometheus_config.h"
#include "labels.h"
#include "mqtt_json_handler.h"
#include "prometheus_metric_query.h"

namespace yafiyogi::mqtt_bridge {

using MetricsJsonPointer = prometheus_detail::MetricsJsonPointer;

class JsonVisitor:
      public MetricsJsonPointer::visitor_type
{
  public:
    constexpr JsonVisitor() noexcept = default;
    constexpr JsonVisitor(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor(JsonVisitor &&) noexcept = default;
    constexpr ~JsonVisitor() noexcept = default;

    constexpr JsonVisitor & operator=(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor & operator=(JsonVisitor &&) noexcept = default;

    [[nodiscard]]
    constexpr Labels & labels() noexcept
    {
      return m_labels;
    }

    [[nodiscard]]
    constexpr const Labels & labels() const noexcept
    {
      return m_labels;
    }

    constexpr void apply(const MetricsJsonPointer::scope_type & /* scope */,
                         MetricsJsonPointer::value_type & metrics,
                         std::string_view str) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(m_labels, str);
      }
    }

    constexpr void apply(const MetricsJsonPointer::scope_type & /* scope */,
                         MetricsJsonPointer::value_type & metrics,
                         std::string_view raw,
                         std::int64_t /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(m_labels, raw);
      }
    }

    constexpr void apply(const MetricsJsonPointer::scope_type & /* scope */,
                         MetricsJsonPointer::value_type & metrics,
                         std::string_view raw,
                         std::uint64_t /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(m_labels, raw);
      }
    }

    constexpr void apply(const MetricsJsonPointer::scope_type & /* scope */,
                         MetricsJsonPointer::value_type & metrics,
                         std::string_view raw,
                         double /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(m_labels, raw);
      }
    }

    constexpr void apply(const MetricsJsonPointer::scope_type & /* scope */,
                         MetricsJsonPointer::value_type & metrics,
                         bool flag) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(m_labels, flag ? g_true_str : g_false_str);
      }
    }

  private:
    constexpr static std::string_view g_true_str{"true"};
    constexpr static std::string_view g_false_str{"false"};

    Labels m_labels;
};

MqttJsonHandler::MqttJsonHandler(std::string_view p_handler_id,
                                 const JsonParserOptions & p_json_options,
                                 JsonParserConfig && p_json_handler_config) noexcept:
  MqttHandler(p_handler_id, type::Json),
  m_parser(p_json_options, std::move(p_json_handler_config))
{
  auto visitor = std::make_unique<JsonVisitor>();
  YY_ASSERT(visitor);
  m_json_visitor = visitor.get();

  auto handler_visitor = yy_util::static_unique_cast<MetricsJsonPointer::visitor_type>(std::move(visitor));
  YY_ASSERT(handler_visitor);
  m_parser.handler().set_visitor(std::move(handler_visitor));
}

void MqttJsonHandler::Event(std::string_view p_topic,
                            const yy_mqtt::TopicLevels & p_topic_levels,
                            std::string_view p_data,
                            const std::string & p_ts) noexcept
{
  spdlog::debug("  handler [{}]", Id());

  auto & labels = m_json_visitor->labels();
  //labels.clear();
  labels.set_label(g_label_path, p_topic_levels);
  labels.set_label(g_label_timestamp, p_ts);
  labels.set_label(g_label_topic, p_topic);

  boost::json::error_code ec{};
  m_parser.write_some(false, p_data.data(), p_data.size(), ec);
}

} // namespace yafiyogi::mqtt_bridge

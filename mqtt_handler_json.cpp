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

#include <cstdint>

#include <memory>
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_assert.h"

#include "yy_prometheus/yy_prometheus_labels.h"
#include "yy_prometheus/yy_prometheus_metric_data.h"

#include "mqtt_handler.h"

#include "mqtt_handler_json.h"

namespace yafiyogi::mqtt_bridge {

using MetricsJsonPointer = prometheus::MetricsJsonPointer;

class JsonVisitor:
      public MetricsJsonPointer::visitor_type
{
  public:
    using size_type = yy_prometheus::MetricDataVector::size_type;

    constexpr JsonVisitor(size_type p_metric_count) noexcept
    {
      m_metric_data.reserve(p_metric_count);
    }

    constexpr JsonVisitor() noexcept = default;
    constexpr JsonVisitor(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor(JsonVisitor &&) noexcept = default;
    constexpr ~JsonVisitor() noexcept = default;

    constexpr JsonVisitor & operator=(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor & operator=(JsonVisitor &&) noexcept = default;

    void labels(const prometheus::Labels * p_labels) noexcept
    {
      if(nullptr == p_labels)
      {
        p_labels = &g_empty_labels;
      }
      m_labels = p_labels;
    }

    void apply_str(const MetricsJsonPointer::scope_type & /* scope */,
                   MetricsJsonPointer::value_type & metrics,
                   std::string_view str) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(str, *m_labels, m_metric_data);
      }
    }

    void apply_int64(const MetricsJsonPointer::scope_type & /* scope */,
                     MetricsJsonPointer::value_type & metrics,
                     std::string_view raw,
                     std::int64_t /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(raw, *m_labels, m_metric_data);
      }
    }

    void apply_uint64(const MetricsJsonPointer::scope_type & /* scope */,
                      MetricsJsonPointer::value_type & metrics,
                      std::string_view raw,
                      std::uint64_t /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(raw, *m_labels, m_metric_data);
      }
    }

    void apply_double(const MetricsJsonPointer::scope_type & /* scope */,
                      MetricsJsonPointer::value_type & metrics,
                      std::string_view raw,
                      double /* num */) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(raw, *m_labels, m_metric_data);
      }
    }

    void apply_bool(const MetricsJsonPointer::scope_type & /* scope */,
                    MetricsJsonPointer::value_type & metrics,
                    bool flag) override
    {
      for(auto & metric : metrics)
      {
        metric->Event(flag ? g_true_str : g_false_str, *m_labels, m_metric_data);
      }
    }

    constexpr void reset() noexcept
    {
      m_metric_data.clear(yy_data::ClearAction::Keep);
    }

    constexpr const yy_prometheus::MetricDataVector & metric_data() const noexcept
    {
      return m_metric_data;
    }

  private:
    constexpr static std::string_view g_true_str{"true"};
    constexpr static std::string_view g_false_str{"false"};

    const yy_prometheus::Labels * m_labels = &g_empty_labels;
    static const yy_prometheus::Labels g_empty_labels;
    yy_prometheus::MetricDataVector m_metric_data{};
};

const prometheus::Labels JsonVisitor::g_empty_labels;

MqttJsonHandler::MqttJsonHandler(std::string_view p_handler_id,
                                 const JsonParserOptions & p_json_options,
                                 JsonPointerConfig && p_json_handler_config,
                                 size_type p_metric_count) noexcept:
  MqttHandler(p_handler_id, type::Json),
  m_parser(p_json_options, std::move(p_json_handler_config))
{
  auto visitor = std::make_unique<JsonVisitor>(p_metric_count);
  YY_ASSERT(visitor);
  m_json_visitor = visitor.get();

  auto handler_visitor = yy_util::static_unique_cast<MetricsJsonPointer::visitor_type>(std::move(visitor));
  YY_ASSERT(handler_visitor);
  m_parser.handler().set_visitor(std::move(handler_visitor));
}

const yy_prometheus::MetricDataVector & MqttJsonHandler::Event(std::string_view p_data,
                                                               const prometheus::Labels & p_labels) noexcept
{
  spdlog::debug("  handler [{}]", Id());

  m_json_visitor->reset();
  m_json_visitor->labels(&p_labels);

  m_parser.write_some(false, p_data.data(), p_data.size(), boost::json::error_code{});

  return m_json_visitor->metric_data();
}

} // namespace yafiyogi::mqtt_bridge

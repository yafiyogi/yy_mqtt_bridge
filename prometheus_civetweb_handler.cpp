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

#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_prometheus/yy_prometheus_configure.h"
#include "yy_prometheus/yy_prometheus_metric_format.h"
#include "yy_prometheus/yy_prometheus_labels.h"
#include "yy_prometheus/yy_prometheus_cache.h"

#include "prometheus_civetweb_handler.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

static constexpr const std::string_view g_http_response{"HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\nConnection: close\r\n\r\n"};

PrometheusWebHandler::PrometheusWebHandler(yy_prometheus::MetricDataCachePtr p_metric_cache,
                                           logger_ptr access_log) noexcept:
  yy_web::WebHandler(std::move(access_log)),
  m_metric_cache(std::move(p_metric_cache)),
  m_buffer(8192)
{
}

bool PrometheusWebHandler::DoGet(struct mg_connection * conn,
                                 const struct mg_request_info * /* ri */)
{
  m_buffer.assign(yy_quad::make_const_span(g_http_response));

  std::optional<std::string_view> last_id{};
  std::string_view last_help{};
  yy_prometheus::MetricType last_type{yy_prometheus::MetricType::None};
  yy_prometheus::MetricUnit last_unit{yy_prometheus::MetricUnit::None};

  auto do_serialize_metrics = [this, &last_id, &last_help, &last_type, &last_unit]
                              (const auto & metric) {
    bool new_headers = false;

    if(!last_id.has_value() || (last_id.value() != metric.Id()))
    {
      new_headers = true;
      last_id = metric.Id();
    }

    if(last_type != metric.Type())
    {
      new_headers = true;
      last_type = metric.Type();
    }

    bool new_unit = last_unit != metric.Unit();
    if(new_unit)
    {
      new_headers = true;
      last_unit = metric.Unit();
    }

    if(last_help != metric.Help())
    {
      new_headers = true;
      new_unit = true;
      last_help = metric.Help();
    }

    if(new_headers)
    {
      yy_prometheus::FormatHeaders(m_buffer, metric, new_unit);
    }

    metric.Format(m_buffer);
  };

  if(m_metric_cache)
  {
    m_metric_cache->Visit(do_serialize_metrics);
  }

  mg_write(conn, m_buffer.data(), m_buffer.size());

  return true;
}

} // namespace yafiyogi::mqtt_bridge::prometheus

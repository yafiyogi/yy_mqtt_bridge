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
#include "yy_prometheus/yy_prometheus_labels.h"
#include "yy_prometheus/yy_prometheus_cache.h"

#include "prometheus_civetweb_handler.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

static constexpr const std::string_view g_http_response{"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"};

PrometheusWebHandler::PrometheusWebHandler(yy_prometheus::MetricDataCachePtr p_metric_cache) noexcept:
  CivetHandler(),
  m_metric_cache(std::move(p_metric_cache)),
  m_buffer(8192)
{
}

bool PrometheusWebHandler::handleGet(CivetServer * /* server */,
                                     struct mg_connection * conn)
{
  if(spdlog::level::debug == spdlog::get_level())
  {
    const struct mg_request_info *ri = mg_get_request_info(conn);

    spdlog::debug("PrometheusWebHandler::handleGet()"sv);
    spdlog::debug("\trequest_method = {}"sv, ri->request_method);
    spdlog::debug("\trequest_uri = {}"sv, ri->request_uri);
    spdlog::debug("\tlocal_uri = {}"sv, ri->local_uri);
    spdlog::debug("\thttp_version = {}"sv, ri->http_version);
    //spdlog::debug("\tquery_string = {}"sv, ri->query_string);
    //spdlog::debug("\tremote_user = {}"sv, ri->remote_user);
    spdlog::debug("\tremote_addr = {}"sv, ri->remote_addr);
    spdlog::debug("\tremote_port = {}"sv, ri->remote_port);
    spdlog::debug("\tis_ssl = {}"sv, ri->is_ssl);
    spdlog::debug("\tnum_headers = {}"sv, ri->num_headers);
    if (ri->num_headers > 0) {
      int i;
      for (i = 0; i < ri->num_headers; i++) {
        spdlog::debug("\t\t{} = {}"sv,
                      ri->http_headers[i].name,
                      ri->http_headers[i].value);
      }
    }
    spdlog::debug("");
  }

  m_buffer.assign(yy_quad::make_const_span(g_http_response));


  std::optional<std::string_view> last_id{};
  std::string_view last_help{};
  yy_prometheus::MetricType last_type{yy_prometheus::MetricType::None};
  yy_prometheus::MetricUnit last_unit{yy_prometheus::MetricUnit::None};

  auto do_serialize_metrics = [this, &last_id, &last_help, &last_type, &last_unit]
                              (const auto & metric) {
    bool new_headers = false;

    if(!last_id.has_value() || (last_id.value() != metric.id))
    {
      new_headers = true;
      last_id = metric.id;
    }

    if(last_type != metric.type)
    {
      new_headers = true;
      last_type = metric.type;
    }

    if(last_unit != metric.unit)
    {
      new_headers = true;
      last_unit = metric.unit;
    }

    bool new_unit = false;

    if(last_help != metric.help)
    {
      new_headers = true;
      new_unit = true;
      last_help = metric.help;
    }

    if(new_headers)
    {
      fmt::format_to(std::back_inserter(m_buffer),
                     "# HELP {} {}\x0a"sv,
                     metric.id,
                     metric.help);
      fmt::format_to(std::back_inserter(m_buffer),
                     "# TYPE {} {}\x0a"sv,
                     metric.id,
                     yy_prometheus::decode_metric_type(metric.type));
      if(new_unit)
      {
        fmt::format_to(std::back_inserter(m_buffer),
                       "# UNIT {} {}\x0a"sv,
                       metric.id,
                       yy_prometheus::decode_metric_unit(metric.unit));
      }
    }

    fmt::format_to(std::back_inserter(m_buffer), "{}{{"sv,
                   metric.id);

    bool first = true;
    metric.labels.visit([&first, this](const auto & label, const auto & value) {
      if(!first)
      {
        fmt::format_to(std::back_inserter(m_buffer), ","sv);
      }
      first = false;
      fmt::format_to(std::back_inserter(m_buffer),
                     "{}=\"{}\""sv,
                     label, value);
    });

    fmt::format_to(std::back_inserter(m_buffer),
                   "}} {} {}\x0a"sv,
                   metric.value,
                   metric.timestamp);
  };

  if(m_metric_cache)
  {
    m_metric_cache->Visit(do_serialize_metrics);
  }

  mg_write(conn, m_buffer.data(), m_buffer.size());

  return true;
}


} // namespace yafiyogi::mqtt_bridge::prometheus

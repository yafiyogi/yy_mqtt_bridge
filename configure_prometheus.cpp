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

#include <string>
#include <string_view>

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_utility.h"

#include "yy_prometheus/yy_prometheus_style.h"
#include "yy_prometheus/yy_prometheus_defaults.h"

#include "yy_web/yy_web_server.h"

#include "configure_prometheus.h"
#include "configure_prometheus_metrics.h"
#include "prometheus_config.h"
#include "yaml_util.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

config configure_prometheus(const YAML::Node & yaml_prometheus)
{
  yy_web::WebServer::Options options;

  options.Add(yy_web::WebServer::decode_query_string, yy_web::WebServer::civetweb_options_yes);
  options.Add(yy_web::WebServer::decode_url, yy_web::WebServer::civetweb_options_yes);
  options.Add(yy_web::WebServer::enable_directory_listing, yy_web::WebServer::civetweb_options_no);
  options.Add(yy_web::WebServer::enable_http2, yy_web::WebServer::civetweb_options_yes);
  options.Add(yy_web::WebServer::enable_keep_alive, yy_web::WebServer::civetweb_options_yes);
  options.Add(yy_web::WebServer::keep_alive_timeout_ms, "5000"sv);

  auto port{yaml_get_value(yaml_prometheus["exporter_port"sv],
                           yy_prometheus::prometheus_default_port)};
  options.Add(yy_web::WebServer::listening_ports, port);
  spdlog::info(" Prometheus Port [{}]"sv, port);

  options.Add(yy_web::WebServer::tcp_nodelay, "1"sv);

  std::string metric_style{yy_util::to_lower(yy_util::trim(yaml_get_value(yaml_prometheus["metric_style"sv],
                                                                          yy_prometheus::style_prometheus)))};

  yy_prometheus::set_metric_style(metric_style);

  auto default_timestamp{decode_metric_timestamp(yaml_get_value(yaml_prometheus["timestamps"sv], ""sv))};
  auto metrics = configure_prometheus_metrics(yaml_prometheus["metrics"sv], default_timestamp);

  auto uri{yaml_get_value(yaml_prometheus["exporter_uri"sv],
                           yy_prometheus::prometheus_default_uri_path)};
  spdlog::info(" Prometheus URI  [{}]"sv, uri);

  return config{std::string{uri}, std::move(options), std::move(metrics)};
}

} // namespace yafiyogi::mqtt_bridge::prometheus

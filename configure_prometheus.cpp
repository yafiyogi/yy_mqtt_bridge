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

#include "yaml-cpp/yaml.h"

#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_string_case.h"

#include "configure_prometheus.h"
#include "configure_prometheus_metrics.h"
#include "prometheus_config.h"
#include "prometheus_style.h"
#include "yaml_util.h"

namespace yafiyogi::mqtt_bridge::prometheus {

config configure_prometheus(const YAML::Node & yaml_prometheus)
{
  int port = yaml_get_value(yaml_prometheus["exporter_port"], prometheus_default_port);

  std::string metric_style = yy_util::to_lower(yy_util::trim(yaml_get_value(yaml_prometheus["metric_style"],
                                                                            prometheus::style_prometheus)));

  prometheus::set_metric_style(metric_style);

  auto metrics = configure_prometheus_metrics(yaml_prometheus["metrics"]);

  return config{port, std::move(metrics)};
}

} // namespace yafiyogi::mqtt_bridge::prometheus

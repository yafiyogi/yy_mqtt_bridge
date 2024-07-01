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

#include "yy_cpp/yy_flat_map.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_type_traits.h"
#include "yy_cpp/yy_utility.h"
#include "yy_cpp/yy_vector_util.h"

#include "prometheus_config.h"
#include "configure_prometheus_metrics.h"

#include "mqtt_handler.h"

namespace yafiyogi::mqtt_bridge {
namespace prometheus_detail {
namespace {

static constexpr std::string_view g_type_guage{"gauge"};

struct metric_type_lookup final
{
    constexpr bool operator<(const metric_type_lookup & other) const noexcept
    {
      return name < other.name;
    }

    constexpr bool operator==(const metric_type_lookup & other) const noexcept
    {
      return name == other.name;
    }

    std::string_view name;
    MetricType type;
};

constexpr auto metric_types = yy_util::sort(yy_util::make_array(metric_type_lookup{g_type_guage, MetricType::Guage}));

constexpr auto metric_type_comp = [](const metric_type_lookup & type,
                                     const std::string & target) -> int
{
  return type.name.compare(target);
};

} // anonymous namespace

MetricType decode_metric_type(const std::string_view & p_metric_type_name)
{
  MetricType type = MetricType::Guage;
  const std::string metric_type_name{yy_util::to_lower(yy_util::trim(p_metric_type_name))};

  if(auto [pos, found] = yy_util::find(metric_types,
                                       metric_type_name,
                                       metric_type_comp);
     found)
  {
    type = pos->type;
  }

  return type;
}

} // namespace prometheus_detail

MetricsMap configure_prometheus_metrics(const YAML::Node & yaml_metrics)
{
  MetricsMap metrics{};

  if(yaml_metrics)
  {
    for(const auto & yaml_metric : yaml_metrics)
    {
      auto metric_id = yy_util::trim(yaml_metric["metric"].as<std::string_view>());
      spdlog::debug("Configuring Prometheus Metric [{}] [line {}].",
                    metric_id,
                    yaml_metric.Mark().line + 1);
      MetricType type = prometheus_detail::decode_metric_type(yaml_metric["type"].as<std::string_view>());

      for(const auto & yaml_handler : yaml_metric["handlers"])
      {
        std::string handler_id = yaml_handler["handler_id"].as<std::string>();
        spdlog::debug("\thandler [{}] [line {}].", handler_id, yaml_handler.Mark().line + 1);

        MetricPtr metric;
        if(auto & yaml_value = yaml_handler["value"];
           yaml_value.IsScalar())
        {
          std::string property = yaml_value.as<std::string>();
          spdlog::debug("\tvalue [{}] [line {}].", property, yaml_handler.Mark().line + 1);

          metric = std::make_unique<Metric>(metric_id,
                                            type,
                                            std::move(property));
        }

        if(metric)
        {
          spdlog::debug("\tadd [{}] to [{}] property [{}].",
                        metric->Id(),
                        handler_id,
                        metric->Property());
          [[maybe_unused]]
          auto [metrics_pos, ignore_found] = metrics.emplace(std::move(handler_id),
                                                             Metrics{});

          auto [ignore_key, handler_metrics] = metrics[metrics_pos];

          handler_metrics.emplace_back(std::move(metric));
        }
      }
    }
  }

  return metrics;
}


} // namespace yafiyogi::mqtt_bridge

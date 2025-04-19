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

#include <string>
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_type_traits.h"
#include "yy_cpp/yy_utility.h"
#include "yy_cpp/yy_vector_util.h"
#include "yy_cpp/yy_yaml_util.h"

#include "yy_values/yy_values_metric_id_fmt.hpp"
#include "yy_values/yy_configure_label_actions.hpp"
#include "yy_values/yy_configure_values.hpp"
#include "yy_values/yy_label_action.hpp"
#include "yy_values/yy_label_action_replace_path.hpp"
#include "yy_values/yy_label_action_copy.hpp"
#include "yy_values/yy_label_action_drop.hpp"
#include "yy_values/yy_label_action_keep.hpp"
#include "yy_values/yy_label_action_replace_path.hpp"
#include "yy_values/yy_value_action_keep.hpp"
#include "yy_values/yy_value_action_switch.hpp"

#include "yy_prometheus/yy_prometheus_configure.h"

#include "configure_prometheus_metrics.h"
#include "mqtt_handler.h"
#include "prometheus_config.h"
#include "prometheus_metric.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

MetricsMap configure_prometheus_metrics(const YAML::Node & yaml_metrics,
                                        yy_prometheus::MetricTimestamp p_default_timestamp)
{
  MetricsMap metrics{};

  if(yaml_metrics)
  {
    for(const auto & yaml_metric : yaml_metrics)
    {
      auto & yaml_handlers = yaml_metric["handlers"sv];
      yy_data::flat_set<std::string_view> handlers{};
      handlers.reserve(yaml_handlers.size());

      yy_values::MetricId metric_id{yy_util::trim(yaml_metric["metric"sv].as<std::string_view>())};
      spdlog::info(" Configuring Prometheus Metric [{}]."sv,
                   metric_id);
      spdlog::trace("  [line {}]."sv,
                    yaml_metric.Mark().line + 1);
      auto type{yy_prometheus::decode_metric_type_name(yy_util::yaml_get_optional_value<std::string_view>(yaml_metric["type"sv]))};
      auto unit{yy_prometheus::decode_metric_unit_name(yy_util::yaml_get_optional_value<std::string_view>(yaml_metric["unit"sv]))};

      if(yy_util::yaml_is_sequence(yaml_handlers))
      {
        for(const auto & yaml_handler : yaml_handlers)
        {
          std::string_view handler_id{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_handler["handler_id"sv]))};
          spdlog::info("   handler [{}]:"sv, handler_id);
          spdlog::trace("    [line {}]."sv, yaml_handler.Mark().line + 1);

          auto timestamp{yy_prometheus::decode_metric_timestamp(yy_util::yaml_get_value<std::string_view>(yaml_handler["timestamp"sv], ""sv),
                                                                p_default_timestamp)};
          const auto & yaml_property = yaml_handler["property"sv];

          if(auto [ignore, emplaced] = handlers.emplace(handler_id);
             emplaced)
          {
            auto property_name{yy_util::yaml_get_optional_value<std::string_view>(yaml_property)};

            if(property_name.has_value()
               && !property_name.value().empty())
            {
              spdlog::info("     - value [{}]."sv, property_name.value());
              spdlog::trace("        [line {}]."sv, yaml_property.Mark().line + 1);

              auto create_label_actions = [&yaml_handler]() {
                spdlog::trace("        Create label actions."sv, yaml_handler.Mark().line + 1);
                return yy_values::configure_label_actions(yaml_handler["label_actions"sv]);
              };

              auto create_value_actions = [&yaml_handler]() {
                spdlog::trace("        Create value actions."sv, yaml_handler.Mark().line + 1);
                return yy_values::configure_value_actions(yaml_handler["value_actions"sv]);
              };

              auto create_property_actions = [&yaml_handler]() {
                spdlog::trace("        Create property actions."sv, yaml_handler.Mark().line + 1);
                return yy_values::configure_property_actions(yaml_handler);
              };

              auto metric{std::make_shared<Metric>(yy_values::MetricId{metric_id},
                                                   std::string{property_name.value()},
                                                   type,
                                                   unit,
                                                   timestamp,
                                                   create_label_actions(),
                                                   create_value_actions(),
                                                   create_property_actions())};

              spdlog::info("     - add metric [{}] to handler [{}] property [{}]."sv,
                           metric->Id(),
                           handler_id,
                           metric->Property());

              auto [metrics_pos, ignore_found] = metrics.emplace(std::string{handler_id},
                                                                 Metrics{});

              auto [ignore_key, handler_metrics] = metrics[metrics_pos];

              handler_metrics.emplace_back(std::move(metric));
            }
          }
          else
          {
            spdlog::warn("   'property' setting!"sv);
            spdlog::trace("    [line {}]."sv, yaml_metric.Mark().line + 1);
          }
        }
      }
    }
  }

  return metrics;
}

} // namespace yafiyogi::mqtt_bridge::prometheus

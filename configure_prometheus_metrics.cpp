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

#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_type_traits.h"
#include "yy_cpp/yy_utility.h"
#include "yy_cpp/yy_vector_util.h"

#include "yy_prometheus/yy_prometheus_configure.h"

#include "configure_prometheus_label_actions.h"
#include "configure_prometheus_metrics.h"
#include "prometheus_label_action.h"
#include "mqtt_handler.h"
#include "prometheus_config.h"
#include "prometheus_label_action_replace_path.h"
#include "prometheus_metric.h"
#include "yaml_util.h"

#include "prometheus_label_action_copy.h"
#include "prometheus_label_action_drop.h"
#include "prometheus_label_action_keep.h"
#include "prometheus_label_action_replace_path.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using namespace std::string_view_literals;

namespace {

enum class ActionType {Copy, Drop, Keep, ReplacePath};

constexpr const auto action_types =
  yy_data::make_lookup<std::string_view, ActionType>({{CopyLabelAction::action_name, ActionType::Copy},
                                                      {DropLabelAction::action_name, ActionType::Drop},
                                                      {KeepLabelAction::action_name, ActionType::Keep},
                                                      {ReplacePathLabelAction::action_name, ActionType::ReplacePath}});

} // anonymous namespace

MetricsMap configure_prometheus_metrics(const YAML::Node & yaml_metrics)
{
  MetricsMap metrics{};

  if(yaml_metrics)
  {
    for(const auto & yaml_metric : yaml_metrics)
    {
      auto & yaml_handlers = yaml_metric["handlers"sv];
      yy_data::flat_set<std::string_view> handlers{};
      handlers.reserve(yaml_handlers.size());

      auto metric_id = yy_util::trim(yaml_metric["metric"sv].as<std::string_view>());
      spdlog::info(" Configuring Prometheus Metric [{}]."sv,
                   metric_id);
      spdlog::debug("  [line {}]."sv,
                    yaml_metric.Mark().line + 1);
      auto type = yy_prometheus::decode_metric_type_name(yaml_get_optional_value(yaml_metric["type"sv], ""sv));
      auto unit = yy_prometheus::decode_metric_unit_name(yaml_get_optional_value(yaml_metric["unit"sv], ""sv));

      for(const auto & yaml_handler : yaml_handlers)
      {
        std::string_view handler_id{yy_util::trim(yaml_get_value(yaml_handler["handler_id"sv], ""sv))};
        spdlog::info("   handler [{}]:"sv, handler_id);
        spdlog::debug("    [line {}]."sv, yaml_handler.Mark().line + 1);

        if(auto [ignore, emplaced] = handlers.emplace(handler_id);
           emplaced)
        {
          const auto & yaml_value = yaml_handler["value"sv];
          auto property = yaml_get_optional_value(yaml_value, "");

          if(property.has_value() && !property.value().empty())
          {
            spdlog::info("     - value [{}]."sv, property.value());
            spdlog::debug("        [line {}]."sv, yaml_value.Mark().line + 1);

            const auto & yaml_label_actions = yaml_handler["label_actions"sv];

            LabelActions actions;
            actions.reserve(yaml_label_actions.size());

            for(const auto & yaml_label_action : yaml_label_actions)
            {
              auto action_name = yy_util::to_lower(yy_util::trim(yaml_get_value(yaml_label_action["action"sv], ""sv)));
              LabelActionPtr action;

              spdlog::info("       - action [{}]."sv, action_name);
              spdlog::debug("          [line {}]."sv, yaml_label_action.Mark().line + 1);
              switch(action_types.lookup(action_name, ActionType::Keep))
              {
                case ActionType::Copy:
                {
                  std::string_view source{yy_util::trim(yaml_get_value(yaml_label_action["source"sv], ""sv))};
                  std::string_view target{yy_util::trim(yaml_get_value(yaml_label_action["target"sv], ""sv))};
                  if(!source.empty()
                     || !target.empty())
                  {
                    action = yy_util::static_unique_cast<LabelAction>(std::make_unique<CopyLabelAction>(std::string{source},
                                                                                                        std::string{target}));
                  }
                }
                break;

                case ActionType::Drop:
                {
                  std::string_view target{yy_util::trim(yaml_get_value(yaml_label_action["target"sv], ""sv))};
                  if(!target.empty())
                  {
                    action = yy_util::static_unique_cast<LabelAction>(std::make_unique<DropLabelAction>(std::string{target}));
                  }
                }
                break;

                case ActionType::Keep:
                {
                  std::string_view target{yy_util::trim(yaml_get_value(yaml_label_action["target"sv], ""sv))};
                  if(!target.empty())
                  {
                    action = yy_util::static_unique_cast<LabelAction>(std::make_unique<KeepLabelAction>(std::string{target}));
                  }
                }
                break;

                case ActionType::ReplacePath:
                {
                  std::string_view target{yy_util::trim(yaml_get_value(yaml_label_action["target"sv], ""sv))};

                  if(!target.empty())
                  {
                    auto topics = configure_label_action_replace_path(yaml_label_action["replace"sv]);

                    action = yy_util::static_unique_cast<LabelAction>(std::make_unique<ReplacePathLabelAction>(std::string{target}, std::move(topics)));
                  }
                }
                break;

                default:
                  spdlog::warn("Unrecognized action [{}]"sv, action_name);
                  spdlog::debug("  [line {}]."sv, yaml_label_action.Mark().line + 1);
                  break;
              }

              if(action)
              {
                actions.emplace_back(std::move(action));
              }
            }

            if(!actions.empty())
            {
              auto metric = std::make_shared<Metric>(metric_id,
                                                     type,
                                                     unit,
                                                     std::string{property.value()},
                                                     std::move(actions));

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
        }
        else
        {
          spdlog::warn("   'property' setting!"sv);
          spdlog::debug("    [line {}]."sv, yaml_metric.Mark().line + 1);
        }
      }
    }
  }

  return metrics;
}


} // namespace yafiyogi::mqtt_bridge::prometheus

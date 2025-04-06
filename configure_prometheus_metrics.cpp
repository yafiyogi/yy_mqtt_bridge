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

#include "yy_values/yy_configure_label_actions.hpp"
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

namespace {

enum class LabelActionType {Copy, Drop, Keep, ReplacePath};

constexpr const auto g_label_action_types =
  yy_data::make_lookup<std::string_view, LabelActionType>(LabelActionType::Keep,
                                                          {{yy_values::CopyLabelAction::action_name, LabelActionType::Copy},
                                                           {yy_values::DropLabelAction::action_name, LabelActionType::Drop},
                                                           {yy_values::KeepLabelAction::action_name, LabelActionType::Keep},
                                                           {yy_values::ReplacePathLabelAction::action_name, LabelActionType::ReplacePath}});

enum class ValueActionType {Keep, Switch};

constexpr const auto g_value_action_types =
  yy_data::make_lookup<std::string_view, ValueActionType>({{yy_values::KeepValueAction::action_name, ValueActionType::Keep},
                                                           {yy_values::SwitchValueAction::action_name, ValueActionType::Switch}});

yy_values::LabelActions configure_metric_label_actions(const YAML::Node & yaml_label_actions)
{
  yy_values::LabelActions label_actions;
  label_actions.reserve(yaml_label_actions.size());

  for(const auto & yaml_label_action : yaml_label_actions)
  {
    auto action_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["action"sv])))};
    yy_values::LabelActionPtr action;

    spdlog::info("       - label action [{}]."sv, action_name);
    spdlog::trace("          [line {}]."sv, yaml_label_action.Mark().line + 1);
    switch(g_label_action_types.lookup(action_name))
    {
      case LabelActionType::Copy:
      {
        std::string_view source{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["source"sv]))};
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!source.empty()
           || !target.empty())
        {
          action = yy_util::static_unique_cast<yy_values::LabelAction>(std::make_unique<yy_values::CopyLabelAction>(std::string{source},
                                                                                                                    std::string{target}));
        }
      }
      break;

      case LabelActionType::Drop:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!target.empty())
        {
          action = yy_util::static_unique_cast<yy_values::LabelAction>(std::make_unique<yy_values::DropLabelAction>(std::string{target}));
        }
      }
      break;

      case LabelActionType::Keep:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!target.empty())
        {
          action = yy_util::static_unique_cast<yy_values::LabelAction>(std::make_unique<yy_values::KeepLabelAction>(std::string{target}));
        }
      }
      break;

      case LabelActionType::ReplacePath:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};

        if(!target.empty())
        {
          auto create_topics = [&yaml_label_action]() {
            return yy_values::configure_label_action_replace_path(yaml_label_action["replace"sv]);
          };

          action = yy_util::static_unique_cast<yy_values::LabelAction>(std::make_unique<yy_values::ReplacePathLabelAction>(std::string{target}, create_topics()));
        }
      }
      break;

      default:
        spdlog::warn("Unrecognized action [{}]"sv, action_name);
        spdlog::trace("  [line {}]."sv, yaml_label_action.Mark().line + 1);
        break;
    }

    if(action)
    {
      label_actions.emplace_back(std::move(action));
    }
  }

  return label_actions;
}

yy_values::ValueActions configure_metric_value_actions(const YAML::Node & yaml_value_actions)
{
  yy_values::ValueActions value_actions;
  if(!yaml_value_actions)
  {
    return value_actions;
  }

  value_actions.reserve(yaml_value_actions.size());

  for(const auto & yaml_value_action : yaml_value_actions)
  {
    auto action_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_value_action["action"sv])))};
    yy_values::ValueActionPtr action;

    spdlog::info("       - value action [{}]."sv, action_name);
    spdlog::trace("          [line {}]."sv, yaml_value_action.Mark().line + 1);
    switch(g_value_action_types.lookup(action_name, ValueActionType::Keep))
    {
      case ValueActionType::Keep:
        // Don't add as it does nothing.
        // action = yy_util::static_unique_cast<ValueAction>(std::make_unique<KeepValueAction>());
        break;

      case ValueActionType::Switch:
      {
        std::optional<std::string> default_value{std::nullopt};

        if(auto & yaml_default = yaml_value_action["default"sv];
           yaml_default)
        {
          default_value = yy_util::yaml_get_optional_value<std::string>(yaml_default);
          if(default_value.has_value())
          {
            spdlog::info("         - default: [{}]", default_value.value());
          }
        }

        yy_values::SwitchValueAction::Switch switch_values;
        auto & yaml_mappings = yaml_value_action["mappings"sv];

        if(auto num_cases = yaml_mappings.size();
           num_cases > 1)
        {
          switch_values.reserve(num_cases - 1);
        }

        for(auto & yaml_case : yaml_mappings)
        {
          if(auto & yaml_input = yaml_case.first;
             yaml_input)
          {
            if(auto & yaml_output = yaml_case.second;
               yaml_output)
            {
              auto input{yy_util::yaml_get_value<std::string_view>(yaml_input)};
              auto output{yy_util::yaml_get_value<std::string_view>(yaml_output)};

              spdlog::info("         - input: [{}] output: [{}]", input, output);
              switch_values.emplace(std::string{input},
                                    std::string{output});
            }
          }
        }

        if(default_value.has_value()
           && !switch_values.empty())
        {
          action = yy_util::static_unique_cast<yy_values::ValueAction>(std::make_unique<yy_values::SwitchValueAction>(std::move(default_value.value()),
                                                                                                                      std::move(switch_values)));
        }
        else
        {
          spdlog::warn(" Value action [{}] not created default {}present, values {}present.",
                       action_name,
                       (default_value.has_value() ? ""sv : "not "sv),
                       (switch_values.empty() ? "not "sv : ""sv) );
        }
      }
      break;

      default:
        spdlog::debug("configure_metric_value_actions(): 4c");
        spdlog::warn("Unrecognized action [{}]"sv, action_name);
        spdlog::trace("  [line {}]."sv, yaml_value_action.Mark().line + 1);
        break;
    }

    if(action)
    {
      value_actions.emplace_back(std::move(action));
    }
  }

  return value_actions;
}


constexpr const auto g_timestamp_types =
  yy_data::make_lookup<std::string_view,
                       yy_prometheus::MetricTimestamp>(yy_prometheus::MetricTimestamp::On,
                                                       {{"on"sv, yy_prometheus::MetricTimestamp::On},
                                                        {"off"sv, yy_prometheus::MetricTimestamp::Off}});

} // anonymous namespace

yy_prometheus::MetricTimestamp decode_metric_timestamp(std::string_view p_timestamp,
                                                       yy_prometheus::MetricTimestamp p_default_timestamp)
{
  return g_timestamp_types.lookup(yy_util::to_lower(yy_util::trim(p_timestamp)),
                                  p_default_timestamp);

}

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

      auto metric_id{yy_util::trim(yaml_metric["metric"sv].as<std::string_view>())};
      spdlog::info(" Configuring Prometheus Metric [{}]."sv,
                   metric_id);
      spdlog::trace("  [line {}]."sv,
                    yaml_metric.Mark().line + 1);
      auto type{yy_prometheus::decode_metric_type_name(yy_util::yaml_get_optional_value<std::string_view>(yaml_metric["type"sv]))};
      auto unit{yy_prometheus::decode_metric_unit_name(yy_util::yaml_get_optional_value<std::string_view>(yaml_metric["unit"sv]))};

      for(const auto & yaml_handler : yaml_handlers)
      {
        std::string_view handler_id{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_handler["handler_id"sv]))};
        spdlog::info("   handler [{}]:"sv, handler_id);
        spdlog::trace("    [line {}]."sv, yaml_handler.Mark().line + 1);

        auto timestamp{decode_metric_timestamp(yy_util::yaml_get_value<std::string_view>(yaml_handler["timestamp"sv], ""sv),
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
              return configure_metric_label_actions(yaml_handler["label_actions"sv]);
            };

            auto create_value_actions = [&yaml_handler]() {
              return configure_metric_value_actions(yaml_handler["value_actions"sv]);
            };

            auto metric{std::make_shared<Metric>(metric_id,
                                                 type,
                                                 unit,
                                                 timestamp,
                                                 std::string{property_name.value()},
                                                 create_label_actions(),
                                                 create_value_actions())};

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

  return metrics;
}


} // namespace yafiyogi::mqtt_bridge::prometheus

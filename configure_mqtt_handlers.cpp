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

#include "boost/json/basic_parser_impl.hpp"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_vector.h"
#include "yy_cpp/yy_vector_util.h"

#include "yy_json/yy_json_pointer.h"
#include "yy_json/yy_json_pointer_util.h"

#include "configure_mqtt_handlers.h"
#include "prometheus_config.h"
#include "mqtt_handler.h"
#include "mqtt_json_handler.h"
#include "mqtt_value_handler.h"

namespace yafiyogi::mqtt_bridge {
namespace {

const boost::json::parse_options g_json_options{ .numbers = boost::json::number_precision::none};

constexpr std::string_view g_type_json{"json"};
constexpr std::string_view g_type_value{"value"};
constexpr std::string_view g_type_text{"text"};

struct handler_type_lookup final
{
    constexpr bool operator<(const handler_type_lookup & other) const noexcept
    {
      return name < other.name;
    }

    constexpr bool operator==(const handler_type_lookup & other) const noexcept
    {
      return name == other.name;
    }

    std::string_view name;
    MqttHandler::type type;
};

constexpr auto handler_type_comp = [](const handler_type_lookup & handler_type,
                                      const std::string & target) -> int
{
  return handler_type.name.compare(target);
};

constexpr auto handler_types = yy_util::sort(yy_util::make_array(handler_type_lookup{g_type_json, MqttHandler::type::Json},
                                                                 handler_type_lookup{g_type_text, MqttHandler::type::Text},
                                                                 handler_type_lookup{g_type_value, MqttHandler::type::Value}));


MqttHandler::type decode_type(const YAML::Node & yaml_type)
{
  MqttHandler::type handler_type = MqttHandler::type::Json;
  if(yaml_type && yaml_type.IsScalar())
  {
    std::string type_name = yy_util::to_lower(yy_util::trim(yaml_type.as<std::string_view>()));

    if(auto [pos, found] = yy_util::find(handler_types,
                                         type_name,
                                         handler_type_comp);
       found)
    {
      handler_type = pos->type;
    }
  }

  return handler_type;
}

MqttHandlerPtr configure_json_property(std::string_view p_id,
                                       const YAML::Node & yaml_json_handler,
                                       MetricsMap & prometheus_metrics)
{
  MqttHandlerPtr mqtt_json_handler{};
  auto yaml_properties = yaml_json_handler["properties"];
  if(yaml_properties && (0 != yaml_properties.size()))
  {
    prometheus_detail::MetricsJsonPointerBuilder json_pointer_builder{};
    bool has_metrics = false;
    std::string_view json_pointer{};
    std::string_view property{};
    yy_json::PathLevels path{};

    auto do_add_property = [&property, &json_pointer, &json_pointer_builder, &has_metrics]
                           (Metrics * visitor_prometheus_metrics, auto /* pos */) {
      if(nullptr != visitor_prometheus_metrics)
      {
        auto [builder_metrics, added] = json_pointer_builder.add_pointer(json_pointer,
                                                                         Metrics{});
        if(nullptr != builder_metrics)
        {
          for(auto & metric : *visitor_prometheus_metrics)
          {
            if(metric && (metric->Property() == property))
            {
              has_metrics = true;
              spdlog::info("       metric [{}] added.",
                           metric->Id());
              builder_metrics->emplace_back(std::move(metric));
            }
          }
        }
      }
    };

    yy_data::flat_set<std::string_view> properties{};
    properties.reserve(yaml_properties.size());
    for(const auto & yaml_property : yaml_properties)
    {
      if(yaml_property.IsScalar())
      {
        json_pointer = yy_util::trim(yaml_property.as<std::string_view>());
        path = yy_json::json_pointer_tokenize(yy_json::json_pointer_trim(json_pointer));
        property = path.back();
      }
      else
      {
        json_pointer = yy_json::json_pointer_trim(yaml_property["json"].as<std::string_view>());
        property = yy_util::trim(yaml_property["id"].as<std::string_view>());
      }
      spdlog::info("     - property [{}] path=[{}]:",
                   property,
                   json_pointer);
      spdlog::debug("        [line {}].",
                    yaml_property.Mark().line + 1);
      if(!json_pointer.empty()
         && !property.empty())
      {
        if(auto [ignore_1, inserted] = properties.emplace(property);
           inserted)
        {
          [[maybe_unused]]
          auto ignore_2 = prometheus_metrics.find_value(do_add_property, p_id);
        }
      }
    }

    if(has_metrics)
    {
      auto config{json_pointer_builder.create(g_json_options.max_depth)};

      mqtt_json_handler = std::make_unique<MqttJsonHandler>(p_id,
                                                            g_json_options,
                                                            std::move(config));
    }
  }

  return mqtt_json_handler;
}

MqttHandlerPtr configure_text_property(std::string_view p_id,
                                       const YAML::Node & /* yaml_text_handler */,
                                       MetricsMap & /* prometheus_metrics */)
{
  return std::make_unique<MqttHandler>(p_id,
                                       MqttHandler::type::Text);
}

MqttHandlerPtr configure_value_property(std::string_view p_id,
                                        const YAML::Node & /* yaml_value_handler */,
                                        MetricsMap & /* prometheus_metrics */)
{
  return std::make_unique<MqttValueHandler>(p_id);
}

} // anonymous namespace

MqttHandlerStore configure_mqtt_handlers(const YAML::Node & yaml_handlers,
                                         prometheus_config & prometheus_config)
{
  if(!yaml_handlers.IsSequence())
  {
    spdlog::error("Configuration error: expecting a sequence of handlers!");

    return MqttHandlerStore{};
  }

  MqttHandlerStore handler_store{};
  handler_store.reserve(yaml_handlers.size());

  for(const auto & yaml_handler: yaml_handlers)
  {
    std::string_view l_id{yy_util::trim(yaml_handler["id"].as<std::string_view>())};
    const MqttHandler::type type = decode_type(yaml_handler["type"]);

    spdlog::info(" Configuring MQTT Handler id [{}]:", l_id);
    spdlog::debug("  [line {}].", yaml_handler.Mark().line + 1);
    MqttHandlerPtr handler;

    spdlog::info("   - type [{}]", yaml_handler["type"].as<std::string_view>());
    switch(type)
    {
      case MqttHandler::type::Json:
        handler = configure_json_property(l_id, yaml_handler, prometheus_config.metrics);
        break;

      case MqttHandler::type::Text:
        handler = configure_text_property(l_id, yaml_handler, prometheus_config.metrics);
        break;

      case MqttHandler::type::Value:
        handler = configure_value_property(l_id, yaml_handler, prometheus_config.metrics);
        break;
    }

    if(handler)
    {
      if(auto id = handler->Id();
         !id.empty())
      {
        if(auto [handler_pos, emplaced] = handler_store.emplace(std::move(id), std::move(handler));
           !emplaced)
        {
          auto [key, ignore] = handler_store[handler_pos];
          spdlog::warn("Handler id [{}] already created. Ignoring [line {}]", key, yaml_handler.Mark().line + 1);
        }
      }
    }
    else
    {
      spdlog::warn("MQTT Handler id [{}] not created!", l_id);
    }
  }

  return handler_store;
}

} // namespace yafiyogi::mqtt_bridge

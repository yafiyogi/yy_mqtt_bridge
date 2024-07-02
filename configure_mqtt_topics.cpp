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

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "yy_cpp/yy_binary_search.h"
#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_vector_util.h"

#include "yy_mqtt/yy_mqtt_util.h"
#include "yy_mqtt/yy_mqtt_faster_topics.h"

#include "configure_mqtt_topics.h"
#include "mqtt_topics.h"
#include "mqtt_handler.h"

namespace yafiyogi::mqtt_bridge {

mqtt_topics configure_mqtt_topics(const YAML::Node & yaml_topics,
                                  const MqttHandlerStore & handlers_store)
{
  yy_quad::simple_vector<std::string> subscriptions{};
  TopicsConfig topics_config{};

  spdlog::info("Configuring topics.");
  for(const auto & yaml_topic: yaml_topics)
  {
    spdlog::debug("Configuring Topic [line {}].", yaml_topic.Mark().line + 1);
    if(auto yaml_handlers = yaml_topic["handlers"];
       yaml_handlers)
    {
      MqttHandlerList mqtt_handlers;
      auto do_add_handler = [&mqtt_handlers](auto * mqtt_handler, auto /* pos */) {
        mqtt_handlers.emplace_back(mqtt_handler->get());
      };

      for(const auto & yaml_handler : yaml_handlers)
      {
        auto handler_id = yy_util::trim(yaml_handler.as<std::string_view>());

        [[maybe_unused]]
        bool found = handlers_store.find_value(do_add_handler, handler_id).found;
      }

      if(!mqtt_handlers.empty())
      {
        auto yaml_subscriptions = yaml_topic["subscriptions"];
        yy_data::flat_set<std::string_view> filters{};
        filters.reserve(yaml_subscriptions.size());

        for(const auto & yaml_subscription : yaml_subscriptions)
        {
          spdlog::debug("Configuring Subscription [line {}].", yaml_subscription.Mark().line + 1);
          if(auto topic = yy_mqtt::topic_trim(yaml_subscription.as<std::string_view>());
             yy_mqtt::topic_validate(topic, yy_mqtt::TopicType::Filter))
          {
            filters.emplace(topic);
          }
        }

        subscriptions.reserve(subscriptions.size() + filters.size());
        for(std::size_t idx = 0; idx < filters.size(); ++idx)
        {
          auto filter = filters[idx];
          spdlog::info("   - subscribing to topic [{}]", filter);
          if(mqtt_handlers.size() == 1)
          {
            spdlog::info("     with handler:");
          }
          else if(mqtt_handlers.size() > 1)
          {
            spdlog::info("     with handlers [{}]:", mqtt_handlers.size());
          }

          for(const auto & handler : mqtt_handlers)
          {
            spdlog::info("     * [{}]", handler->Id());
          }

          yy_mqtt::faster_topics_add(topics_config, filter, MqttHandlerList{mqtt_handlers});
          subscriptions.emplace_back(std::string{filter});
        }
      }
    }
  }

  yy_util::sort(subscriptions);
  yy_util::unique(subscriptions);

  return mqtt_topics{std::move(subscriptions), topics_config.create_automaton()};
}

} // namespace yafiyogi::mqtt_bridge

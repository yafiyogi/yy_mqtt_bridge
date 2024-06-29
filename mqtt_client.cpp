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

#include "yy_mqtt/yy_mqtt_util.h"

#include "mqtt_handler.h"
#include "configure_mqtt.h"

#include "mqtt_client.h"

namespace yafiyogi::mqtt_bridge {

mqtt_client::mqtt_client(mqtt_config & p_config):
  mosqpp::mosquittopp(),
  m_topics(std::move(p_config.topics)),
  m_subscriptions(std::move(p_config.subscriptions)),
  m_id(std::move(p_config.id)),
  m_host(std::move(p_config.host)),
  m_port(p_config.port),
  m_is_connected(false)
{
}

void mqtt_client::connect()
{
  int mqtt_version = MQTT_PROTOCOL_V5;
  mosqpp::mosquittopp::opts_set(MOSQ_OPT_PROTOCOL_VERSION, &mqtt_version);

  int nodelay_flag = 1;
  mosqpp::mosquittopp::opts_set(MOSQ_OPT_TCP_NODELAY, &nodelay_flag);

  int quickack_flag = 1;
  mosqpp::mosquittopp::opts_set(MOSQ_OPT_TCP_QUICKACK, &quickack_flag);

  mosqpp::mosquittopp::connect(m_host.c_str(), m_port, 60);
}

void mqtt_client::on_connect(int rc)
{
  m_is_connected = true;

  spdlog::debug("{}[{}]", "MQTT Connected status=", rc);
  spdlog::info("{}", "Subscribing to:");
  yy_quad::simple_vector<char *> subs;
  subs.reserve(m_subscriptions.size());

  for(auto & sub : m_subscriptions)
  {
    spdlog::info("{}[{}]", "\t - ", sub);

    subs.emplace_back(sub.data());
  }

  subscribe_multiple(NULL, static_cast<int>(subs.size()), subs.data(), 0, 0);
}

void mqtt_client::on_subscribe(int /* mid */,
                               int /* qos_count */,
                               const int * /* granted_qos */)
{
  spdlog::info("MQTT Subscribed.");
}

void mqtt_client::on_disconnect(int rc)
{
  spdlog::info("{}[{}]", "MQTT Disconnected status=", rc);
  m_is_connected = false;
}

void mqtt_client::on_message(const struct mosquitto_message * message)
{
  std::string_view topic{yy_mqtt::topic_trim(message->topic)};
  if(auto payloads = m_topics.find(topic);
     !payloads.empty())
  {
    spdlog::debug("Processing [{}] payloads=[{}]", topic, payloads.size());
    yy_mqtt::TopicLevels topic_levels{yy_mqtt::topic_tokenize(topic)};

    for(const auto & handlers : payloads)
    {
      spdlog::info("[{}]:", topic);
      for(auto & handler : *handlers)
      {
        std::string_view msg{static_cast<std::string_view::value_type *>(message->payload),
                             static_cast<std::string_view::size_type>(message->payloadlen)};
        handler->Event(topic, topic_levels, msg);
      }
    }
  }
}

} // namespace yafiyogi::mqtt_bridge

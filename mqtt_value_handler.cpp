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

#include "mqtt_value_handler.h"

namespace yafiyogi::mqtt_bridge {

MqttValueHandler::MqttValueHandler(std::string_view p_handler_id,
                                   prometheus::Metrics && p_metrics) noexcept:
  MqttHandler(p_handler_id, type::Value),
  m_metrics(std::move(p_metrics))
{
}

void MqttValueHandler::Event(std::string_view p_data,
                        const prometheus::Labels & p_labels) noexcept
{
  for(auto & metric : m_metrics)
  {
    metric->Event(p_data, p_labels);
  }
}

} // namespace yafiyogi::mqtt_bridge

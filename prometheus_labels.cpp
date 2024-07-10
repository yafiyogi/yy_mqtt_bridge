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

#include "prometheus_labels.h"

namespace yafiyogi::mqtt_bridge::prometheus {
namespace {

const std::string g_empty_str{};

} // anonymous namespace

void Labels::clear() noexcept
{
  m_labels.clear();
  m_path.clear();
}

void Labels::set_label(std::string_view p_label,
                       std::string_view p_value)
{
  auto do_set_value = [p_value](auto value, auto) {
    *value = p_value;
  };

  if(auto [pos, found] = m_labels.find_value(do_set_value, p_label);
     !found)
  {
    if(auto [label, value, inserted] = m_labels.add_empty(pos);
       inserted)
    {
      *label = p_label;
      *value = p_value;
    }
  }
}

void Labels::set_path(const yy_mqtt::TopicLevels & p_path) noexcept
{
  m_path = p_path;
}

const std::string & Labels::get_label(const std::string_view p_label) const noexcept
{
  const std::string * label = &g_empty_str;

  auto do_get_value = [&label](const std::string * p_visitor_label, auto) {
    if(nullptr != p_visitor_label)
    {
      label = p_visitor_label;
    }
  };

  [[maybe_unused]]
  auto ignore = m_labels.find_value(do_get_value, p_label);

  return *label;
}

} // namespace yafiyogi::mqtt_bridge::prometheus

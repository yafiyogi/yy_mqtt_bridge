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

#include <cstddef>

#include <string>
#include <memory>

#include "yy_cpp/yy_variant_util.h"
#include "yy_prometheus/yy_prometheus_labels.h"
#include "label_action.h"

#include "replacement_format.h"

#include "label_action_replace_path.h"

namespace yafiyogi::mqtt_bridge {

ReplacePathLabelAction::ReplacePathLabelAction(std::string && p_label_name,
                                               ReplacementTopics && p_topics) noexcept:
  m_label_name(std::move(p_label_name)),
  m_label_value(),
  m_topics(std::move(p_topics))
{
}

void ReplacePathLabelAction::Apply(const yy_prometheus::Labels & /* labels */,
                                   yy_prometheus::Labels & metric_labels) noexcept
{
  if(auto payloads = m_topics.find(metric_labels.get_label(yy_prometheus::g_label_topic));
     !payloads.empty())
  {
    const auto & levels = metric_labels.get_path();

    for(const auto * replacements : payloads)
    {
      for(const auto & format : *replacements)
      {
        m_label_value.clear();

        for(const auto & fmt_elem : format)
        {
          std::visit([&levels, this](const auto & formatter) { formatter(levels, m_label_value); },
                     fmt_elem);
        }

        metric_labels.set_label(m_label_name, m_label_value);
      }
    }
  }
}


} // namespace yafiyogi::mqtt_bridge
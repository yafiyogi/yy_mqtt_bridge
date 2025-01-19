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

#pragma once

#include <limits>
#include <memory>
#include <optional>

#include "yy_cpp/yy_vector.h"

#include "yy_mqtt/yy_mqtt_state_topics.h"

#include "label_action.h"
#include "replacement_format.h"

namespace yafiyogi::mqtt_bridge {

using ReplacementTopicsConfig = yy_mqtt::state_topics<ReplaceFormats>;
using ReplacementTopics = ReplacementTopicsConfig::automaton_type;

class ReplacePathLabelAction:
      public LabelAction
{
  public:
    explicit ReplacePathLabelAction(std::string && p_label_name,
                                    ReplacementTopics && p_topics) noexcept;
    constexpr ReplacePathLabelAction() noexcept = default;
    constexpr ReplacePathLabelAction(const ReplacePathLabelAction &) noexcept = default;
    constexpr ReplacePathLabelAction(ReplacePathLabelAction &&) noexcept = default;

    constexpr ReplacePathLabelAction & operator=(const ReplacePathLabelAction &) noexcept = default;
    constexpr ReplacePathLabelAction & operator=(ReplacePathLabelAction &&) noexcept = default;

    void Apply(const yy_prometheus::Labels & /* labels */,
               const yy_mqtt::TopicLevelsView & /* p_levels */,
               yy_prometheus::Labels & /* metric_labels */) noexcept override;

    static constexpr const std::string_view action_name{"replace-path"};
    constexpr std::string_view Name() const noexcept override
    {
      return action_name;
    }

  private:
    std::string m_label_name{};
    std::string m_label_value{};
    ReplacementTopics m_topics{};
};

} // namespace yafiyogi::mqtt_bridge

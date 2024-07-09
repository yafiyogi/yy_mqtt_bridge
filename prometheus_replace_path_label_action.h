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

#include "yy_mqtt/yy_mqtt_faster_topics.h"

#include "prometheus_label_action.h"

namespace yafiyogi::mqtt_bridge::prometheus {

struct PathReplaceElement
{
    using size_type = uint32_t;
    static constexpr size_type no_param = std::numeric_limits<size_type>::max();

    std::string prefix;
    size_type idx = no_param;
};

using ReplaceFormat = yy_quad::simple_vector<PathReplaceElement>;
using ReplaceFormats = yy_quad::simple_vector<ReplaceFormat>;

using ReplacementTopicsConfig = yy_mqtt::faster_topics<ReplaceFormats>;
using ReplacementTopics = ReplacementTopicsConfig::automaton;

class ReplacePathLabelAction:
      public LabelAction
{
  public:
    explicit ReplacePathLabelAction(std::string && p_label_name,
                                    ReplacementTopics && p_topics) noexcept;
    constexpr ReplacePathLabelAction() noexcept = default;
    constexpr ReplacePathLabelAction(const ReplacePathLabelAction &) noexcept = default;
    constexpr ReplacePathLabelAction(ReplacePathLabelAction &&) noexcept = default;
    constexpr ~ReplacePathLabelAction() noexcept override = default;

    constexpr ReplacePathLabelAction & operator=(const ReplacePathLabelAction &) noexcept = default;
    constexpr ReplacePathLabelAction & operator=(ReplacePathLabelAction &&) noexcept = default;

    void Apply(const Labels & /* labels */, Labels & /* metric_labels */) noexcept override;

  private:
    std::string m_label_name;
    std::string m_label_value;
    ReplacementTopics m_topics;
};

} // namespace yafiyogi::mqtt_bridge::prometheus
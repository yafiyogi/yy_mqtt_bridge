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

#pragma once

#include <limits>
#include <memory>
#include <optional>

#include "yy_cpp/yy_vector.h"

#include "label_action.h"

namespace yafiyogi::mqtt_bridge {

class KeepLabelAction final:
      public LabelAction
{
  public:
    explicit KeepLabelAction(std::string && p_label) noexcept;
    constexpr KeepLabelAction() noexcept = default;
    constexpr KeepLabelAction(const KeepLabelAction &) noexcept = default;
    constexpr KeepLabelAction(KeepLabelAction &&) noexcept = default;

    constexpr KeepLabelAction & operator=(const KeepLabelAction &) noexcept = default;
    constexpr KeepLabelAction & operator=(KeepLabelAction &&) noexcept = default;

    void Apply(const yy_prometheus::Labels & /* labels */,
               const yy_mqtt::TopicLevelsView & /* p_levels */,
               yy_prometheus::Labels & /* metric_labels */) noexcept override;

    static constexpr const std::string_view action_name{"keep"};
    constexpr std::string_view Name() const noexcept override
    {
      return action_name;
    }

  private:
    std::string m_label{};
};

} // namespace yafiyogi::mqtt_bridge

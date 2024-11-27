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

#include "label_action.h"

namespace yafiyogi::mqtt_bridge {

class CopyLabelAction:
      public LabelAction
{
  public:
    explicit CopyLabelAction(std::string && p_label_source,
                             std::string && p_label_target) noexcept;
    constexpr CopyLabelAction() noexcept = default;
    constexpr CopyLabelAction(const CopyLabelAction &) noexcept = default;
    constexpr CopyLabelAction(CopyLabelAction &&) noexcept = default;

    constexpr CopyLabelAction & operator=(const CopyLabelAction &) noexcept = default;
    constexpr CopyLabelAction & operator=(CopyLabelAction &&) noexcept = default;

    void Apply(const yy_prometheus::Labels & /* labels */,
               yy_prometheus::Labels & /* metric_labels */) noexcept override;

    static constexpr const std::string_view action_name{"copy"};
    constexpr std::string_view Name() const noexcept override
    {
      return action_name;
    }

  private:
    std::string m_label_source{};
    std::string m_label_target{};
};

} // namespace yafiyogi::mqtt_bridge
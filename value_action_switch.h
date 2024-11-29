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

#include <initializer_list>

#include "value_action.h"

namespace yafiyogi::mqtt_bridge {

class SwitchValueAction:
      public ValueAction
{
  public:
    using Switch = yy_data::flat_map<std::string, std::string>;

    constexpr SwitchValueAction(Switch && p_switch) noexcept:
      m_switch(std::move(p_switch))
    {
    }

    constexpr SwitchValueAction() noexcept = default;
    constexpr SwitchValueAction(const SwitchValueAction &) noexcept = default;
    constexpr SwitchValueAction(SwitchValueAction &&) noexcept = default;

    constexpr SwitchValueAction & operator=(const SwitchValueAction &) noexcept = default;
    constexpr SwitchValueAction & operator=(SwitchValueAction &&) noexcept = default;

    void Apply(yy_prometheus::MetricData & p_metric_data,
               ValueType p_value_type) noexcept override;

    static constexpr const std::string_view action_name{"switch"};
    std::string_view Name() const noexcept override
    {
      return action_name;
    }

  private:

    Switch m_switch;
};

} // namespace yafiyogi::mqtt_bridge

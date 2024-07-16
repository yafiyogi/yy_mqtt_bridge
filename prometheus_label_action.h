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

#include "yy_prometheus/yy_prometheus_labels_fwd.h"

#include "prometheus_label_action_fwd.h"

namespace yafiyogi::mqtt_bridge::prometheus {

using Labels = yy_prometheus::Labels;

class LabelAction
{
  public:
    constexpr LabelAction() noexcept = default;
    constexpr LabelAction(const LabelAction &) noexcept = default;
    constexpr LabelAction(LabelAction &&) noexcept = default;
    constexpr virtual ~LabelAction() noexcept = default;

    constexpr LabelAction & operator=(const LabelAction &) noexcept = default;
    constexpr LabelAction & operator=(LabelAction &&) noexcept = default;

    virtual void Apply(const Labels & /* labels */, Labels & /* metric_labels */) noexcept = 0;
    virtual std::string_view Name() const noexcept = 0;
};

} // namespace yafiyogi::mqtt_bridge::prometheus

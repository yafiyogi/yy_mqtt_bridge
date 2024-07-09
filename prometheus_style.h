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

#include <chrono>
#include <memory>
#include <string_view>

namespace yafiyogi::mqtt_bridge::prometheus {

inline constexpr std::string_view style_prometheus{"prometheus"};
inline constexpr std::string_view style_open_metric{"open-metric"};

struct metric_style
{
    using clock_type = std::chrono::system_clock;

    constexpr metric_style() noexcept = default;
    constexpr metric_style(const metric_style &) noexcept = default;
    constexpr metric_style(metric_style &&) noexcept = default;
    constexpr virtual ~metric_style() noexcept = default;

    constexpr metric_style & operator=(const metric_style &) noexcept = default;
    constexpr metric_style & operator=(metric_style &&) noexcept = default;


    virtual int64_t timestamp(clock_type::time_point time) const noexcept = 0;
};
using metric_style_ptr = std::unique_ptr<metric_style>;

struct prometheus_style:
      metric_style
{
    constexpr int64_t timestamp(clock_type::time_point time_point) const noexcept override
    {
      return std::chrono::time_point_cast<std::chrono::milliseconds>(time_point).time_since_epoch().count();
    }
};

struct open_metric_style:
      metric_style
{
    constexpr int64_t timestamp(clock_type::time_point time_point) const noexcept override
    {
      return std::chrono::time_point_cast<std::chrono::seconds>(time_point).time_since_epoch().count();
    }
};

void set_metric_style(std::string_view metric_style_name) noexcept;
metric_style & get_metric_style() noexcept;

} // namespace yafiyogi::mqtt_bridge::prometheus

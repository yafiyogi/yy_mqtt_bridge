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

#include <mutex>
#include <string>
#include <string_view>

#include "yy_cpp/yy_make_lookup.h"

#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "prometheus_style.h"

namespace yafiyogi::mqtt_bridge::prometheus {
namespace {

enum class Style { Prometheus, OpenMetric };

constexpr std::string_view g_style_prometheus{"prometheus"};
constexpr std::string_view g_style_open_metric_1{"openmetric"};
constexpr std::string_view g_style_open_metric_2{"open metric"};
constexpr std::string_view g_style_open_metric_3{"open_metric"};

constexpr auto styles =
  yy_data::make_lookup<std::string_view, Style>({{style_prometheus, Style::Prometheus},
                                                 {style_open_metric, Style::OpenMetric},
                                                 {g_style_open_metric_1, Style::OpenMetric},
                                                 {g_style_open_metric_2, Style::OpenMetric},
                                                 {g_style_open_metric_3, Style::OpenMetric}});

metric_style_ptr create_style(std::string_view p_style_name)
{
  metric_style_ptr style;

  std::string style_name{yy_util::to_lower(yy_util::trim(p_style_name))};

  switch(styles.lookup(style_name, Style::Prometheus))
  {
    case Style::Prometheus:
      style = std::make_unique<prometheus_style>();
      break;

    case Style::OpenMetric:
      style = std::make_unique<open_metric_style>();
      break;
  }

  return style;
}

static std::once_flag g_metric_style_flag;
static metric_style_ptr g_metric_style;
static std::once_flag g_metric_style_name_flag;
static std::string g_metric_style_name{g_style_prometheus};

const std::string & get_metric_style_name()
{
  std::call_once(g_metric_style_name_flag, []() {
  });

  return g_metric_style_name;
}

} // anonymous namespace

void set_metric_style(std::string_view p_metric_style_name) noexcept
{
  std::call_once(g_metric_style_name_flag, [p_metric_style_name]() {
    g_metric_style_name = p_metric_style_name;
  });
}

metric_style & get_metric_style() noexcept
{
  std::call_once(g_metric_style_flag, []() {
    g_metric_style = create_style(get_metric_style_name());
  });

  return *g_metric_style;
}

} // namespace yafiyogi::mqtt_bridge::prometheus

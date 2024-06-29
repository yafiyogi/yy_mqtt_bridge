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

#include <string_view>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "logger.h"

namespace yafiyogi::mqtt_bridge {
namespace {

constexpr std::string_view g_std_err{"stderr"};
constexpr std::string_view g_log_name{"mqtt_bridge_log"};
constexpr std::string_view g_default_file_path{"./mqtt_bridge.log"};

std::mutex g_logger_mtx{};
logger_ptr g_logger = spdlog::stderr_color_mt("stderr");

void set_g_logger(logger_ptr log)
{
  std::unique_lock lck(g_logger_mtx);

  g_logger = log;
  spdlog::set_default_logger(g_logger);
}

} // anonymous namespace

void set_logger(std::string_view file_path)
{
  stop_log();

  set_g_logger(spdlog::daily_logger_mt(g_log_name.data(), file_path.data(), 0, 0));
}

void set_logger()
{
  set_logger(g_default_file_path);
}

void set_console_logger()
{
  if(!spdlog::get(std::string{g_std_err}))
  {
    set_g_logger(spdlog::stderr_color_mt("stderr"));
  }
}

logger_ptr get_log()
{
  std::unique_lock lck(g_logger_mtx);

  return g_logger;
}

void stop_log()
{
  stop_log(g_log_name);

  std::unique_lock lck(g_logger_mtx);
  g_logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_default_logger(g_logger);
}

void stop_log(std::string_view logger_name)
{
  spdlog::drop(logger_name.data());
}

void stop_all_logs()
{
  std::unique_lock lck(g_logger_mtx);
  g_logger = logger_ptr{};

  spdlog::drop_all();
}

} // namespace yafiyogi::mqtt_bridge

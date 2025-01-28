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

#include <cstdint>

#include <limits>
#include <string>
#include <string_view>
#include <variant>

#include "yy_cpp/yy_vector.h"
#include "yy_mqtt/yy_mqtt_types.h"

namespace yafiyogi::mqtt_bridge {
namespace replacement_format_detail {

using size_type = uint32_t;

} // namespace replacement_format_detail

class FormatPrefix final
{
  public:
    FormatPrefix(std::string_view p_prefix) noexcept:
      m_prefix(p_prefix)
    {
    }

    FormatPrefix() noexcept = default;
    FormatPrefix(const FormatPrefix &) noexcept = default;
    FormatPrefix(FormatPrefix &&) noexcept = default;

    FormatPrefix & operator=(const FormatPrefix &) noexcept = default;
    FormatPrefix & operator=(FormatPrefix &&) noexcept = default;

    void operator()(const yy_mqtt::TopicLevelsView & /* p_path */,
                    std::string & label_value) const noexcept;

  private:
    std::string m_prefix{};
};

class FormatLevel final
{
  public:
    FormatLevel(std::string_view p_prefix, size_type p_idx) noexcept:
      m_prefix(p_prefix),
      m_idx(p_idx)
    {
    }

    FormatLevel() noexcept = default;
    FormatLevel(const FormatLevel &) noexcept = default;
    FormatLevel(FormatLevel &&) noexcept = default;

    FormatLevel & operator=(const FormatLevel &) noexcept = default;
    FormatLevel & operator=(FormatLevel &&) noexcept = default;

    void operator()(const yy_mqtt::TopicLevelsView & p_path,
                    std::string & label_value) const noexcept;

  private:
    std::string m_prefix{};
    size_type m_idx = 0;
};

using PathReplaceElement = std::variant<FormatPrefix, FormatLevel>;

using ReplaceFormat = yy_quad::simple_vector<PathReplaceElement>;
using ReplaceFormats = yy_quad::simple_vector<ReplaceFormat>;

} // namespace yafiyogi::mqtt_bridge

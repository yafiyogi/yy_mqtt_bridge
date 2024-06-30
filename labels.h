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

#include <string>
#include <string_view>

#include "yy_cpp/yy_flat_map.h"

#include "yy_mqtt/yy_mqtt_types.h"

namespace yafiyogi::mqtt_bridge {

inline constexpr const std::string_view g_label_help{"help"};
inline constexpr const std::string_view g_label_path{"path"};
inline constexpr const std::string_view g_label_timestamp{"timestamp"};
inline constexpr const std::string_view g_label_topic{"topic"};

class Labels final
{
  public:
    constexpr Labels() noexcept = default;
    constexpr Labels(const Labels &) noexcept = default;
    constexpr Labels(Labels &&) noexcept = default;
    constexpr ~Labels() noexcept = default;

    constexpr Labels & operator=(const Labels &) noexcept = default;
    constexpr Labels & operator=(Labels &&) noexcept = default;

    void clear() noexcept;
    void set_label(std::string_view p_label,
                   std::string_view p_value);
    void set_label(std::string_view p_label,
                   const yy_mqtt::TopicLevels & p_path) noexcept;

    [[nodiscard]]
    const std::string & get_label(const std::string & p_label) const noexcept;

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
      return m_labels.size();
    }

    constexpr bool operator<(const Labels & other) const noexcept
    {
      return std::tie(m_path, m_labels) < std::tie(other.m_path, other.m_labels);
    }

    constexpr bool operator==(const Labels & other) const noexcept
    {
      return std::tie(m_path, m_labels) == std::tie(other.m_path, other.m_labels);
    }

  private:
    using LabelStore = yy_data::flat_map<std::string, std::string, yy_data::ClearAction::Keep, yy_data::ClearAction::Keep>;
    LabelStore m_labels;
    yy_mqtt::TopicLevels m_path;
};

} // namespace yafiyogi::mqtt_bridge

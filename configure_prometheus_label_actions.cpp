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

#include <string>

#include "absl/strings/string_view.h"
#include "re2/re2.h"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "yy_cpp/yy_fast_atoi.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_util.h"

#include "yy_mqtt/yy_mqtt_util.h"

#include "configure_prometheus_label_actions.h"
#include "prometheus_label_action.h"
#include "replacement_format.h"
#include "yaml_util.h"

namespace yafiyogi::mqtt_bridge::prometheus {
namespace {

// '([^\\]*)' -> capture everything before '\'
// '(?:\\(?:(\d{1,2})|(.)))?' -> capture optional index or escaped character.
// '(\d{1,2})' -> capture index: '\1'. Min one character, max two characters.
// '(.)' -> capture escaped character: '\p'
const re2::RE2 re_idx{R"(([^\\]*)(?:\\(?:(\d{1,2})|(.)))?)", RE2::Quiet};

constexpr auto escapes =
  yy_data::make_lookup<std::string_view, std::string_view>({{"n", "\n"},
                                                            {"t", "\t"}});

} // namespace


template<typename Visitor>
void configure_label_action_replace_format(std::string_view replacement_format,
                                           Visitor && visitor)
{
  if(!replacement_format.empty())
  {
    ReplaceFormat format;
    absl::string_view config_format{replacement_format.data(), replacement_format.size()};

    format.clear();
    std::string_view prefix;
    std::string_view escape;
    std::string_view idx;
    std::string format_prefix;

    while(RE2::Consume(&config_format, re_idx, &prefix, &idx, &escape))
    {
      format_prefix.append(prefix);
      if(!idx.empty())
      {
        if(auto [format_idx, state] = yy_util::fast_atoi<PathReplaceElement::size_type>(idx);
           (yy_util::FastFloatRV::Ok == state) && (format_idx > 0))
        {
          format.emplace_back(PathReplaceElement{std::string{format_prefix}, format_idx - 1});
          format_prefix.clear();
        }
        else
        {
          spdlog::warn("Format error: error with format [{}].",
                       replacement_format);
        }
      }
      else if(!escape.empty())
      {
        format_prefix.append(escapes.lookup(escape, escape));
      }

      if(config_format.empty())
      {
        break;
      }
    }

    if(!format_prefix.empty())
    {
      format.emplace_back(PathReplaceElement{std::move(format_prefix), PathReplaceElement::no_param});
    }

    if(!format.empty())
    {
      visitor(format);
    }
  }
}

ReplacementTopics configure_label_action_replace_path(const YAML::Node & yaml_replace)
{
  ReplacementTopicsConfig topics_config;

  for(const auto & yaml_format : yaml_replace)
  {
    if(yaml_format)
    {
      std::string_view replacement_pattern{"#"};
      std::string_view replacement_format{};

      if(yaml_is_scalar(yaml_format))
      {
        replacement_format = yaml_get_value(yaml_format, "");
      }
      else
      {
        replacement_pattern = yy_util::trim(yaml_get_value(yaml_format["pattern"], "#"));
        replacement_format = yy_util::trim(yaml_get_value(yaml_format["format"], ""));
      }

      if(yy_mqtt::topic_validate(replacement_pattern, yy_mqtt::TopicType::Filter))
      {
        configure_label_action_replace_format(replacement_format,
                                              [replacement_pattern, &topics_config](ReplaceFormat & format)
                                              {
                                                auto [topic_formats, ignore] = yy_mqtt::faster_topics_add(topics_config, replacement_pattern, ReplaceFormats{});

                                                topic_formats->emplace_back(format);
                                              });
      }
    }
  }

  return topics_config.create_automaton();
}


} // namespace yafiyogi::mqtt_bridge::prometheus

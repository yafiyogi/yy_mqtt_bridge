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

#include "re2/re2.h"

namespace yafiyogi::mqtt_bridge {
namespace {

re2::RE2 g_re_idx{R"(([^\\]*)(?:\\(?:(\d{2})|(.)))?)"};
// '([^\\]*)' -> capture everything before '\'
// '(?:\\(?:(\d{2})|(.)))?' -> capture optional index or optional escaped character.
// '(\d+)' -> capture index min 1 digit, max 2 digits: '\1'
// '(.)' -> capture escaped character: '\p'

static constexpr std::string_view escapes{"nt"};

} // anonymous namespace

void create_replace_formats(std::string_view source,
                            std::string_view format,
                            std::string_view regex)
{
  if(label_path == source)
  {
  }
  else
  {
    if(regex.empty())
    {
      regex = R"((.*?))";
    }

    auto create_regex = [regex]() {
      return re2::RE2(absl::string_view{regex.data(), regex.size()});
    };
  }
}

} // namespace yafiyogi::mqtt_bridge

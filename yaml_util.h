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

#include <string_view>

#include "yy_cpp/yy_type_traits.h"
#include "yy_cpp/yy_string_traits.h"

#include "yaml_fwd.h"

namespace yafiyogi::mqtt_bridge {

template<typename T,
         std::enable_if_t<!yy_traits::is_std_string_v<T>
                          && !yy_traits::is_c_string_v<T>,
                          bool> = true>
auto yaml_get_value(const YAML::Node & node, T value)
{
  if(node && node.IsScalar())
  {
    value = node.as<T>();
  }

  return value;
}

template<typename T,
         std::enable_if_t<yy_traits::is_std_string_v<T>
                          || yy_traits::is_c_string_v<T>,
                          bool> = true>
auto yaml_get_value(const YAML::Node & node, T value)
{
  return yaml_get_value(node, std::string_view{value});
}

template<typename T>
auto yaml_get_value(const YAML::Node & node)
{
  return yaml_get_value(node, T{});
}

} // namespace yafiyogi::mqtt_bridge
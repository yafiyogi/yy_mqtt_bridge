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

#include <memory>

#include "CivetServer.h"

#include "yy_cpp/yy_vector.h"

namespace yafiyogi::yy_prometheus {

class MetricDataCache;
using MetricDataCachePtr = std::shared_ptr<MetricDataCache>;

} // namespace yafiyogi::yy_prometheus

namespace yafiyogi::mqtt_bridge::prometheus {

class PrometheusWebHandler:
      public CivetHandler
{
  public:
    explicit PrometheusWebHandler(yy_prometheus::MetricDataCachePtr p_metric_cache) noexcept;

    PrometheusWebHandler() noexcept = default;
    PrometheusWebHandler(const PrometheusWebHandler &) noexcept = default;
    PrometheusWebHandler(PrometheusWebHandler &&) noexcept = default;
    ~PrometheusWebHandler() noexcept override = default;

    PrometheusWebHandler & operator=(const PrometheusWebHandler &) noexcept = default;
    PrometheusWebHandler & operator=(PrometheusWebHandler &&) noexcept = default;

    bool handleGet(CivetServer *server, struct mg_connection *conn) override;

  private:
    using metric_buffer = yy_quad::simple_vector<char>;

    yy_prometheus::MetricDataCachePtr m_metric_cache{};
    metric_buffer m_buffer{};
};

using PrometheusWebHandlerPtr = std::unique_ptr<PrometheusWebHandler>;

} // namespace yafiyogi::mqtt_bridge::prometheus

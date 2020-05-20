/***************************************************************************
 *
 * Prohject
 *    ______                      _        _____
 *   |  ____|                    | |      |  __ \
 *   | |__ _   _ _ __  _ __   ___| |______| |__) | __ _____  ___   _
 *   |  __| | | | '_ \| '_ \ / _ \ |______|  ___/ '__/ _ \ \/ / | | |
 *   | |  | |_| | | | | | | |  __/ |      | |   | | | (_) >  <| |_| |
 *   |_|   \__,_|_| |_|_| |_|\___|_|      |_|   |_|  \___/_/\_\\__, |
 *                                                              __/ |
 *                                                             |___/
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_funnel_proxy_handler_HttpProxyHandler_hpp
#define oatpp_funnel_proxy_handler_HttpProxyHandler_hpp

#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"

#include "oatpp/core/macro/component.hpp"

namespace oatpp { namespace funnel_proxy { namespace handler {

class HttpProxyHandler : public oatpp::web::server::HttpRequestHandler {
private:
  static bool shouldHaveBody(const Headers& headers);
  static Headers proxyHeaders(const Headers& headers);
private:
  OATPP_COMPONENT(std::shared_ptr<web::client::HttpRequestExecutor>, m_requestExecutor);
public:

  async::CoroutineStarterForResult<const std::shared_ptr<OutgoingResponse>&>
  handleAsync(const std::shared_ptr<IncomingRequest>& request);

};

}}}

#endif // oatpp_funnel_proxy_handler_HttpProxyHandler_hpp

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

#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"
#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"

#include "oatpp/core/macro/component.hpp"

class AppComponent {
public:

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([] {
    return std::make_shared<oatpp::async::Executor>();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
    return oatpp::network::server::SimpleTCPConnectionProvider::createShared(9000);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor); // get Async executor component
    return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::client::HttpRequestExecutor>, httpRequestExecutor)([] {
    auto connectionProvider = oatpp::network::client::SimpleTCPConnectionProvider::createShared("localhost", 8000);
    auto connectionPool = std::make_shared<oatpp::network::ClientConnectionPool>(connectionProvider, 10, std::chrono::seconds(5));
    return oatpp::web::client::HttpRequestExecutor::createShared(connectionPool);
  }());

};

#endif /* AppComponent_hpp */

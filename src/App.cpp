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

#include "./AppComponent.hpp"
#include "handler/HttpProxyHandler.hpp"

#include "oatpp/network/server/Server.hpp"
#include <iostream>

void run() {
  
  AppComponent components; // Create scope Environment components

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  auto proxyHandler = std::make_shared<oatpp::funnel_proxy::handler::HttpProxyHandler>();

  router->route("GET",      "*", proxyHandler);
  router->route("HEAD",     "*", proxyHandler);
  router->route("POST",     "*", proxyHandler);
  router->route("PUT",      "*", proxyHandler);
  router->route("DELETE",   "*", proxyHandler);
  router->route("CONNECT",  "*", proxyHandler);
  router->route("OPTIONS",  "*", proxyHandler);
  router->route("TRACE",    "*", proxyHandler);
  router->route("PATCH",    "*", proxyHandler);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, connectionHandler);

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::server::Server server(connectionProvider, connectionHandler);

  std::cout <<
  "  ______                      _        _____                     \n"
  " |  ____|                    | |      |  __ \\                    \n"
  " | |__ _   _ _ __  _ __   ___| |______| |__) | __ _____  ___   _ \n"
  " |  __| | | | '_ \\| '_ \\ / _ \\ |______|  ___/ '__/ _ \\ \\/ / | | |\n"
  " | |  | |_| | | | | | | |  __/ |      | |   | | | (_) >  <| |_| |\n"
  " |_|   \\__,_|_| |_|_| |_|\\___|_|      |_|   |_|  \\___/_/\\_\\\\__, |\n"
  "                                                            __/ |\n"
  "                                                           |___/ "
  "\n"
  "Server is running on port " << components.serverConnectionProvider.getObject()->getProperty("port").toString()->c_str()
  << std::endl;

  /* Run server */
  server.run();
  
}

int main(int argc, const char * argv[]) {
  oatpp::base::Environment::init();
  run();
  oatpp::base::Environment::destroy();
  return 0;
}

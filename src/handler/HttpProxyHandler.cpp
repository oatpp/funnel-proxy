/***************************************************************************
 *
 * Project
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

#include "HttpProxyHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"

namespace oatpp { namespace funnel_proxy { namespace handler {

bool HttpProxyHandler::shouldHaveBody(const Headers& headers) {
  const auto& all = headers.getAll_Unsafe();

  {
    auto it = all.find(Header::CONTENT_LENGTH);
    if(it != all.end()) {
      return true;
    }
  }

  {
    auto it = all.find(Header::TRANSFER_ENCODING);
    if(it != all.end()) {
      return true;
    }
  }

  return false;
}

HttpProxyHandler::Headers HttpProxyHandler::proxyHeaders(const Headers& headers) {
  Headers result;
  for(const auto& h : headers.getAll_Unsafe()) {
    if(h.first != Header::TRANSFER_ENCODING) {
      result.put(h.first, h.second);
    }
  }
  return result;
}

async::CoroutineStarterForResult<const std::shared_ptr<HttpProxyHandler::OutgoingResponse>&>
HttpProxyHandler::handleAsync(const std::shared_ptr<IncomingRequest>& request) {

  /*
   * The proxy coroutine.
   *
   * Request
   * End(A) ---> (Proxy) ---> End(B)
   *                           |
   *                           |
   *                        Response
   * End(A) <--- (Proxy) <--- End(B)
   */
  class HandlerCoroutine : public async::CoroutineWithResult<HandlerCoroutine, const std::shared_ptr<OutgoingResponse>&> {
  private:
    typedef web::protocol::http::incoming::Response IncomingResponse;
  private:
    std::shared_ptr<web::client::HttpRequestExecutor> m_requestExecutor;
    std::shared_ptr<IncomingRequest> m_request;
  private:
    std::shared_ptr<web::client::HttpRequestExecutor::ConnectionHandle> m_connection;
    std::shared_ptr<IncomingResponse> m_clientResponse;
  private:
    oatpp::String m_body;
    bool m_isBodyPresent;
  public:

    HandlerCoroutine(const std::shared_ptr<web::client::HttpRequestExecutor>& requestExecutor,
                     const std::shared_ptr<IncomingRequest>& request)
      : m_requestExecutor(requestExecutor)
      , m_request(request)
    {}

    /*
     * Entrypoint!
     */
    Action act() override {
      m_body = nullptr;
      m_isBodyPresent = shouldHaveBody(m_request->getHeaders());
      if(m_isBodyPresent) {
        return m_request->readBodyToStringAsync().callbackTo(&HandlerCoroutine::onBodyRead_A);
      }
      return yieldTo(&HandlerCoroutine::onClientConnect_B);
    }

    /*
     * Read body from A.
     */
    Action onBodyRead_A(const oatpp::String& body) {
      m_body = body;
      return yieldTo(&HandlerCoroutine::onClientConnect_B);
    }

    /*
     * Connect to B.
     */
    Action onClientConnect_B() {
      return m_requestExecutor->getConnectionAsync().callbackTo(&HandlerCoroutine::onClientConnected_B);
    }

    /*
     * Successfully connected to B.
     */
    Action onClientConnected_B(const std::shared_ptr<web::client::HttpRequestExecutor::ConnectionHandle>& connection) {
      m_connection = connection;
      return yieldTo(&HandlerCoroutine::onClientRequest_B);
    }

    /*
     * Send Request to B.
     */
    Action onClientRequest_B() {
      std::shared_ptr<web::protocol::http::outgoing::Body> body;
      if(m_body) {
        body = web::protocol::http::outgoing::BufferBody::createShared(m_body);
      }
      return m_requestExecutor->executeAsync(
        m_request->getStartingLine().method.toString(),
        m_request->getStartingLine().path.toString(),
        proxyHeaders(m_request->getHeaders()),
        body,
        m_connection
      ).callbackTo(&HandlerCoroutine::onClientResponse_B);
    }

    /*
     * On response from B.
     */
    Action onClientResponse_B(const std::shared_ptr<IncomingResponse>& response) {
      m_clientResponse = response;
      m_body = nullptr;
      m_isBodyPresent = shouldHaveBody(m_clientResponse->getHeaders());
      if(m_isBodyPresent) {
        return m_clientResponse->readBodyToStringAsync().callbackTo(&HandlerCoroutine::onBodyRead_B);
      }
      return yieldTo(&HandlerCoroutine::onClientResponse_A);
    }

    /*
     * Read body from B.
     */
    Action onBodyRead_B(const oatpp::String& body) {
      m_body = body;
      return yieldTo(&HandlerCoroutine::onClientResponse_A);
    }

    /*
     * Send response to A.
     */
    Action onClientResponse_A() {
      if(m_clientResponse->getStatusCode() != 200) {
        m_requestExecutor->invalidateConnection(m_connection);
      }
      if(!m_body) m_body = "";

      auto response = ResponseFactory::createResponse(
        Status(m_clientResponse->getStatusCode(), ""),
        m_body
      );

      for(const auto& h : m_clientResponse->getHeaders().getAll_Unsafe()) {
        if(h.first != Header::TRANSFER_ENCODING) {
          response->putHeader(h.first.toString(), h.second.toString());
        }
      }
      return _return(response);
    }

    /*
     * Error occured - invalidate pool connection.
     */
    Action handleError(Error* error) override {
      m_requestExecutor->invalidateConnection(m_connection);
      return error;
    }

  };

  return HandlerCoroutine::startForResult(m_requestExecutor, request);

}

}}}

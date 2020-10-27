/*
 *  Project:    moba-display
 *
 *  Copyright (C) 2016 Stefan Paproth <pappi-@gmx.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/agpl.txt>.
 *
 */

#include "webserver.h"
#include "resourceloader.h"

#include "web/server_http.hpp"
#include "web/server_ws.hpp"

#include "moba/systemmessage.h"
#include "moba/timermessage.h"
#include "moba/environmenthandler.h"

#include <future>

#include <unordered_set>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

const size_t delay = 1000; //delay in milliseconds

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

WebServer::WebServer(EndpointPtr endpoint, unsigned short port) : endpoint{endpoint}, port{port} {
}

void WebServer::run() {
    auto io_service = std::make_shared<SimpleWeb::asio::io_service>();
    HttpServer httpServer;
    httpServer.config.port = port;
    httpServer.io_service = io_service;

    WsServer wsServer;
    auto &wsEndpoint = wsServer.endpoint["^/diplay/?$"];
    std::thread msgThread([&,this]() {
        std::shared_ptr<WsServer::OutMessage> out_message;
        while(true) {
            try {
                endpoint->connect();
                endpoint->sendMsg(SystemGetHardwareState{});
                endpoint->sendMsg(TimerGetColorTheme{});
                endpoint->sendMsg(EnvGetEnvironment{});
                while(true) {
                    std::promise<void> promise;
                    std::string data = endpoint->waitForNewMsg2();

                    io_service->post([&] {
                        for(auto &a_connection : wsEndpoint.get_connections()) {
                            a_connection->send(data, [](const boost::system::error_code &ec) {}, 130);
                        }
                        promise.set_value();
                    });
                    promise.get_future().wait();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                }
                exit(EXIT_SUCCESS);
            } catch(std::exception &e) {
                //LOG(moba::common::LogLevel::NOTICE) << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(4));
            }
        }
    });
    wsEndpoint.on_message = [this](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
        auto out_message = in_message->string();
        //endpoint->sendMsg();

        std::cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
    };

    ResourceLoader loader;
    httpServer.default_resource["GET"] = loader;

    httpServer.on_upgrade = [&wsServer](std::unique_ptr<SimpleWeb::HTTP> &socket, std::shared_ptr<HttpServer::Request> request) {
        auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
        connection->method = std::move(request->method);
        connection->path = std::move(request->path);
        connection->http_version = std::move(request->http_version);
        connection->header = std::move(request->header);
        wsServer.upgrade(connection);
    };
    httpServer.start();
    io_service->run();
    msgThread.join();


    //system("xdg-open http://localhost:8080/");

}
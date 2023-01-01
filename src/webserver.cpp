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

#include "moba/systemmessages.h"
#include "moba/timermessages.h"
#include "moba/environmentmessages.h"

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
    auto &wsEndpoint = wsServer.endpoint["^/display/?$"];
    std::thread msgThread([&,this]() {
        if(system("xdg-open http://localhost:8080/")) {
            std::cerr << "unable to launch webpage" << std::endl;
        }
        while(true) {
            try {
                endpoint->connect();
                while(true) {
                    std::promise<void> promise;
                    auto data = endpoint->waitForNewMsgAsString();
                    io_service->post([&] {
                        for(auto &a_connection : wsEndpoint.get_connections()) {
                            a_connection->send(data);
                        }
                        promise.set_value();
                    });
                    promise.get_future().wait();
                }
                exit(EXIT_SUCCESS);
            } catch(std::exception &e) {
                //EXC_LOG("std::exception", e.what());
                std::this_thread::sleep_for(std::chrono::seconds(4));
            }
        }
    });

    wsEndpoint.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
        /*
        auto out_message = make_shared<WsServer::OutMessage>();

        out_message->write(image_buffer.data(), image_buffer.size());

        connections_receiving.emplace(connection);
        connection->send(out_message, [&connections_receiving, connection](const boost::system::error_code &ec) {
          connections_receiving.erase(connection);
        }, 130);
         */
    };

    wsEndpoint.on_message = [this](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in) {
        auto out = in->string();

        auto pos = out.find('#');
        if(pos == std::string::npos) {
            return;
        }

        auto pos1 = out.find('#', pos + 1);
        if(pos1 == std::string::npos) {
            return;
        }

        try {
            endpoint->sendMsg(
                std::stoi(out.substr(0, pos)),
                std::stoi(out.substr(pos + 1, pos1 - pos - 1)),
                (pos1 + 1 > out.length()) ? "null" : out.substr(pos1 + 1)
            );
        } catch(std::exception &e) {
            //EXC_LOG("std::exception", e.what());
        }
    };

    wsEndpoint.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec) {
        std::cerr << "Websocket Error: " << ec << ", error message: " << ec.message() << std::endl;
    };


    httpServer.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & ec) {
        std::cerr << "Webserver Error: " << ec << ", error message: " << ec.message() << std::endl;
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
}
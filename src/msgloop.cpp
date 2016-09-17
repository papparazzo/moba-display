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

#include <boost/algorithm/string.hpp>

#include "msgloop.h"

MessageLoop::MessageLoop(
    const std::string &appName, const moba::Version &version,
    unsigned short port, size_t numThreads
) : appName{appName}, version{version}, server{port, numThreads} {
    auto& echo = this->server.endpoint["^/diplay/?$"];
    echo.onopen = [this](std::shared_ptr<WSServer::Connection> con) {
        LOG(moba::INFO) << "opened connection <" << (size_t)con.get() << ">" << std::endl;
        for(auto item : this->msgBuffer) {
            auto send_stream = std::make_shared<WSServer::SendStream>();
            *send_stream << item.second;
            this->server.send(con, send_stream);
        }
    };

    echo.onclose = [](std::shared_ptr<WSServer::Connection> connection, int status, const std::string& /*reason*/) {
        LOG(moba::INFO) << "closed connection <" << (size_t)connection.get() << "> with status code <" << status << ">" << std::endl;
    };

    echo.onerror = [](std::shared_ptr<WSServer::Connection> connection, const boost::system::error_code& ec) {
        LOG(moba::INFO) << "error in connection <" << (size_t)connection.get() << ">. <" << ec << ":" << ec.message() << ">" << std::endl;
    };

    std::thread serverThread([this](){
        this->server.start();
    });
    serverThread.detach();
}

void MessageLoop::run() {
    while(true) {
        moba::MessagePtr msg = this->msgHandler.recieveMsg();
        if(!msg) {
            continue;
        }
        LOG(moba::NOTICE) << "New Message <" << *msg << ">" << std::endl;
        this->msgBuffer[msg->getMsgType()] = msg->getRawMessage();
        this->sendMessage(msg->getRawMessage());
    }
}

void MessageLoop::connect(const std::string &host, int port) {
    LOG(moba::INFO) << "Try to connect (" << host << ":" << port << ")..." << std::endl;
    this->msgHandler.connect(host, port);
    moba::JsonArrayPtr groups(new moba::JsonArray());
    groups->push_back(moba::toJsonStringPtr("BASE"));
    groups->push_back(moba::toJsonStringPtr("ENV"));
    groups->push_back(moba::toJsonStringPtr("SYSTEM"));

    this->appId = this->msgHandler.registerApp(
        this->appName,
        this->version,
        groups
    );
    LOG(moba::NOTICE) << "AppId <" << this->appId << ">" << std::endl;
}

void MessageLoop::init() {
    this->msgHandler.sendGetHardwareState();
    this->msgHandler.sendGetAutoMode();
    this->msgHandler.sendGetColorTheme();
    this->msgHandler.sendGetEnvironment();
/*
    echo.onmessage = [&server](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {
        auto message_str = message->string();
        LOG(moba::INFO) << "message received: <" << message_str << "> from <" << (size_t)connection.get() << ">" << std::endl;



        auto send_stream = make_shared<WsServer::SendStream>();

        *send_stream << message_str;
        //server.send is an asynchronous function
        server.send(connection, send_stream, [](const boost::system::error_code& ec) {
            if(ec) {
                cout << "Server: Error sending message. Error: " << ec << ", error message: " << ec.message() << endl;
            }
        });
        *send_stream << "blaha";
        cout << "Server: Sending message \"" << message_str <<  "\" to " << (size_t)connection.get() << endl;
        server.send(connection, send_stream);
    };
    */
}

void MessageLoop::sendMessage(const std::string &data) {
    LOG(moba::INFO) << "send data <" << data << "> to connections" << std::endl;
    for(auto con : this->server.get_connections()) {
        auto send_stream = std::make_shared<WSServer::SendStream>();
        *send_stream << data;
        this->server.send(con, send_stream);
    }
}


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

#pragma once

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "web/server_ws.hpp"
#include "moba/endpoint.h"

#include <moba/log.h>

class MessageLoop : private boost::noncopyable {
    public:
        using WSServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

        MessageLoop(
            EndpointPtr endpoint,
            unsigned short port,
            size_t numThreads
        );

        void run();
        void init();

    protected:
        void sendMessage(const std::string &data);

        WSServer server;
        EndpointPtr endpoint;
        std::map<moba::Message::MessageType, std::string> msgBuffer;
};

/*
 *  Project:    moba-display
 *
 *  Copyright (C) 2020 Stefan Paproth <pappi-@gmx.de>
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

#include <boost/filesystem.hpp>
#include <fstream>

#include "web/server_http.hpp"
#include "web/server_ws.hpp"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class ResourceLoader {
public:
    ResourceLoader(const std::string& rootPath = "www-data");

    virtual ~ResourceLoader();

    void operator()(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

protected:

    void readAndSend(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs);

    std::string rootPath;
    std::vector<char> buffer;
};


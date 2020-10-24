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

#include "web/server_http.hpp"
#include <future>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

void runWebServer(unsigned short port) {
    HttpServer server;
    server.config.port = port;
    server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("www-data");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if(
                std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
                !std::equal(web_root_path.begin(), web_root_path.end(), path.begin())
            ) {
                throw std::invalid_argument("path must be within root path");
            }

            if(boost::filesystem::is_directory(path)) {
                path /= "index.html";
            }

            SimpleWeb::CaseInsensitiveMultimap header;

            auto ifs = std::make_shared<std::ifstream>();
            ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

            if(!*ifs) {
                throw std::invalid_argument("could not read file");
            }
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);

            header.emplace("Content-Length", std::to_string(length));
            response->write(header);

            // Trick to define a recursive function within this scope (for example purposes)
            class FileServer {
                public:
                    static void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
                        // Read and send 128 KB at a time
                        static std::vector<char> buffer(131072); // Safe when server is running on one thread
                        std::streamsize read_length;
                        if((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) <= 0) {
                            return;
                        }
                        response->write(&buffer[0], read_length);
                        if(read_length != static_cast<std::streamsize>(buffer.size())) {
                            return;
                        }
                        response->send([response, ifs](const SimpleWeb::error_code &ec) {
                            if(!ec) {
                                read_and_send(response, ifs);
                            }
                        });
                    }
            };
            FileServer::read_and_send(response, ifs);

        } catch(const std::exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
        }
    };

    // Start server and receive assigned port when server is listening for requests
    std::promise<unsigned short> server_port;
    std::thread server_thread([&server, &server_port]() {
        // Start server
        server.start([&server_port](unsigned short port) {
            server_port.set_value(port);
        });
    });
    server_thread.join();
}

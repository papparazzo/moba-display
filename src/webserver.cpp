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

WebServer::WebServer(int port) {
    this->server.config.port = 8080;
    
    std::thread serverThread([this]() {
        this->server.start();
    });
    serverThread.detach();

}






//Added for the default_resource example
#include <fstream>
#include <boost/filesystem.hpp>
#include <vector>
#include <algorithm>
#include <string>
#include <exception>




void default_resource_send(
    const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response,
    const std::shared_ptr<std::ifstream> &ifs
) {
    //read and send 128 KB at a time
    static std::vector<char> buffer(131072); // Safe when server is running on one thread
    std::streamsize read_length;
    if((read_length = ifs->read(&buffer[0], buffer.size()).gcount()) > 0) {
        response->write(&buffer[0], read_length);
        if(read_length == static_cast<std::streamsize>(buffer.size())) {
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
                if(!ec){
                    default_resource_send(server, response, ifs);
                } else {
                    std::cerr << "Connection interrupted" << std::endl;
                }
            });
        }
    }
}

int main() {
    server.default_resource["GET"] = [&server](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("src/www-data");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            //Check if path is within web_root_path
            if(std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
               !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw std::invalid_argument("path must be within root path");
            if(boost::filesystem::is_directory(path)) {
                path /= "index.html";
            }
            if(!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path))) {
                throw std::invalid_argument("file does not exist");
            }


            auto ifs = std::make_shared<std::ifstream>();
            ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);
            
            if(!*ifs) {
                throw std::invalid_argument("could not read file");
            }
            auto length=ifs->tellg();
            ifs->seekg(0, std::ios::beg);

            *response << 
                "HTTP/1.1 200 OK\r\n" << 
                "Content-Length: " << length << "\r\n\r\n";
            
            default_resource_send(server, response, ifs);
        } catch(const std::exception &e) {
            std::string content = "Could not open path " + request->path + ": " + e.what();
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
        }
    };
}






WebServer::WebServer(const WebServer& orig) {
}

WebServer::~WebServer() {
}


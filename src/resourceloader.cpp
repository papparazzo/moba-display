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

#include "resourceloader.h"

ResourceLoader::ResourceLoader(const std::string& rootPath) : rootPath{rootPath}, buffer(131072) {
}

ResourceLoader::~ResourceLoader() {
}

void ResourceLoader::operator()(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
    try {
        auto webRootPath = boost::filesystem::canonical(rootPath);
        auto path = boost::filesystem::canonical(webRootPath / request->path);
        // Check if path is within web_root_path
        if(
            std::distance(webRootPath.begin(), webRootPath.end()) > std::distance(path.begin(), path.end()) ||
            !std::equal(webRootPath.begin(), webRootPath.end(), path.begin())
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

        readAndSend(response, ifs);

    } catch(const std::exception &e) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
    }
}

void ResourceLoader::readAndSend(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
    auto length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount();
    if(length <= 0) {
        return;
    }
    response->write(&buffer[0], length);
    if(length != static_cast<std::streamsize>(buffer.size())) {
        return;
    }
    response->send([response, ifs, this](const SimpleWeb::error_code &ec) {
        if(!ec) {
            readAndSend(response, ifs);
        }
    });
}

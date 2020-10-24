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

#include <string>

#include "webserver.h"

#include "moba/socket.h"
#include "moba/endpoint.h"
#include "moba/systemmessage.h"
#include "moba/timermessage.h"
#include "moba/environmenthandler.h"

#include <moba-common/log.h>
#include <moba-common/version.h>
#include <moba-common/helper.h>

#include <config.h>

#include "msgloop.h"

namespace {
    moba::common::AppData appData = {
        PACKAGE_NAME,
        moba::common::Version(PACKAGE_VERSION),
        __DATE__,
        __TIME__,
        "::1",
        7000
    };

    std::string pidfile = "/var/run/" PACKAGE_NAME ".pid";
}

int main(int argc, char *argv[]) {
    printAppData(appData);
    moba::common::setCoreFileSizeToULimit();

    /*
    if(geteuid() != 0) {
        LOG(tlog::Error) << "This daemon can only be run by root user, exiting" << std::endl;
	    exit(EXIT_FAILURE);
	}

    int fh = open(pidfile, O_RDWR | O_CREAT, 0644);

    if(fh == -1) {
        LOG(tlog::Error) << "Could not open PID lock file <" << pidfile << ">, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(lockf(fh, F_TLOCK, 0) == -1) {
        LOG(tlog::Error) << "Could not lock PID lock file <" << pidfile << ">, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }
*/

    runWebServer();

    auto socket = std::make_shared<Socket>(appData.host, appData.port);
    auto endpoint = EndpointPtr{new Endpoint{socket, appData.appName, appData.version, {Message::ENVIRONMENT, Message::SYSTEM}}};

    while(true) {
        try {
            endpoint->connect();
            endpoint->sendMsg(SystemGetHardwareState{});
            endpoint->sendMsg(TimerGetColorTheme{});
            endpoint->sendMsg(EnvGetEnvironment{});
            MessageLoop loop(endpoint , 8008, 4);
            loop.run();
            exit(EXIT_SUCCESS);
        } catch(std::exception &e) {
            LOG(moba::common::LogLevel::NOTICE) << e.what() << std::endl;
            sleep(4);
        }
    }
}

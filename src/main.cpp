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

#include <boost/shared_ptr.hpp>

#include <moba/log.h>
#include <moba/version.h>
#include <moba/helper.h>
#include <moba/helper.h>

#include <config.h>

#include "msgloop.h"

namespace {
    moba::AppData appData = {
        PACKAGE_NAME,
        moba::Version(PACKAGE_VERSION),
        __DATE__,
        __TIME__,
        "localhost",
        7000
    };

    std::string pidfile = "/var/run/environment.pid";
}

int main(int argc, char *argv[]) {
    switch(argc) {
        case 3:
            appData.port = atoi(argv[2]);

        case 2:
            appData.host = std::string(argv[1]);

        default:
            break;
    }
    printAppData(appData);

    if(!moba::setCoreFileSizeToULimit()) {
        LOG(moba::WARNING) << "Could not set corefile-size to unlimited";
    }

    /*
    if(geteuid() != 0) {
        LOG(tlog::Error) << "This daemon can only be run by root user, exiting";
	    exit(EXIT_FAILURE);
	}

    int fh = open(pidfile, O_RDWR | O_CREAT, 0644);

    if(fh == -1) {
        LOG(tlog::Error) << "Could not open PID lock file <" << pidfile << ">, exiting";
        exit(EXIT_FAILURE);
    }

    if(lockf(fh, F_TLOCK, 0) == -1) {
        LOG(tlog::Error) << "Could not lock PID lock file <" << pidfile << ">, exiting";
        exit(EXIT_FAILURE);
    }
*/
    while(true) {
        try {
            MessageLoop loop(appData.appName, appData.version, 8008, 4);
            loop.connect(appData.host, appData.port);
            loop.init();
            loop.run();
            exit(EXIT_SUCCESS);
        } catch(std::exception &e) {
            LOG(moba::NOTICE) << e.what();
            sleep(4);
        }
    }
}

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

hour = 10;
minute = 20;

function handleSystemMessages(msgId, data) {
    switch(msgId) {
        case 5:
            switch(data) {
                case 'MANUEL':
                    $('#emergency-button')
                        .addClass('btn-danger')
                        .removeClass('btn-success')
                        .html('Notaus')
                        .data('msg', '7#2#true');
                    break;

                case 'EMERGENCY_STOP':
                    $('#emergency-button')
                        .addClass('btn-success')
                        .removeClass('btn-danger')
                        .html('Freigabe')
                        .data('msg', '7#2#false');
                    break;


                    //<button id="emergency-button" type="button" class="btn-block btn-lg btn btn-danger">Notaus</button>
                    //<button id="emergency-button" type="button" class="btn-block btn-lg btn btn-danger">Notaus</button>

            }
            break;
    }
}

$(document).ready(function() {
    var ws = new WebSocket("ws://192.168.178.34:8080/display");
    ws.onmessage = function(event) {
        console.debug("WebSocket message received:", event.data);
        var d = JSON.parse(event.data);
        switch(d.groupId) {
            case 7: // SYSTEM
                handleSystemMessages(d.messageId, d.data);
                break;
        }
    };
    ws.onopen = function(event) {
        ws.send('7#4#');
    };

    $('#emergency-button').click(function(){
        ws.send($(this).data('msg'));
    });


/*
    ws.onmessage = function(event) {

        switch(d.msgType) {
            case "GLOBAL_TIMER_EVENT":
                var t = d.msgData.curModelTime.split(" ");
                var r = t[1].split(":");
                hour = r[0];
                minute = r[1];
                clearInterval(clkID);
                clkID = setInterval(drawClock, (1000 / (d.msgData.multiplicator / 60)));
                $('#time').html(d.msgData.curModelTime + ', Multiplikator: ' + d.msgData.multiplicator);
                break;

            case "COLOR_THEME_EVENT":
                if(d.msgData == "DIM") {
                    $("body, #time, #reload-counter, #data").animate({
                      color: "#fff",
                      backgroundColor: "#333"
                    });
                } else {
                    $("body, #time, #reload-counter, #data").animate({
                      color: "#000",
                      backgroundColor: "#fff"
                    });
                }
                break;

            case "SET_COLOR_THEME":
                $('#brightTime').html(d.msgData.brightTime);
                $('#dimTime').html(d.msgData.dimTime);
                $('#condition').html(d.msgData.condition);
                break;

            case "SET_ENVIRONMENT":
                $('#thunderStorm').html(d.msgData.thunderStorm);
                $('#wind').html(d.msgData.wind);
                $('#rain').html(d.msgData.rain);
                $('#environmentSound').html(d.msgData.environmentSound);
                $('#aux01').html(d.msgData.aux01);
                $('#aux02').html(d.msgData.aux02);
                $('#aux03').html(d.msgData.aux03);
                break;

            default:
                break;
        }
    };
 */
});
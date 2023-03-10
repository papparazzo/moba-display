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

let trains = [];

function handleSystemMessages(msgId, data) {
    switch(msgId) {
        case 5:
            switch(data) {
                case 'MANUEL':
                    $('#status-hw').removeClass().addClass('badge badge-success');
                    $('#status-sw').removeClass().addClass('badge badge-warning');
                    $('#alert-notificaton').html('manuell');
                    $('#emergency-button')
                        .addClass('btn-danger')
                        .removeClass('btn-success btn-secondary')
                        .html('Notaus')
                        .data('msg', '7#2#true')
                        .prop("disabled", false);
                    break;

                case 'EMERGENCY_STOP':
                    $('#status-hw').removeClass().addClass('badge badge-danger');
                    $('#status-sw').removeClass().addClass('badge badge-warning');
                    $('#alert-notificaton').html('Notaus');
                    $('#emergency-button')
                        .addClass('btn-success')
                        .removeClass('btn-danger btn-secondary')
                        .html('Freigabe')
                        .data('msg', '7#2#false')
                        .prop("disabled", false);
                    break;

                case 'ERROR':
                    $('#status-hw').removeClass().addClass('badge badge-danger');
                    $('#status-sw').removeClass().addClass('badge badge-danger');
                    $('#alert-notificaton').html('Fehler');
                    $('#emergency-button')
                        .addClass('btn-secondary')
                        .removeClass('btn-danger btn-success')
                        .prop("disabled", false);
                    break;

                case 'STANDBY':
                    $('#status-hw').removeClass().addClass('badge badge-warning');
                    $('#status-sw').removeClass().addClass('badge badge-warning');
                    $('#alert-notificaton').html('Standby');
                    $('#emergency-button')
                        .addClass('btn-secondary')
                        .removeClass('btn-danger btn-success')
                        .prop("disabled", false);
                    break;

                case 'AUTOMATIC':
                    $('#status-hw').removeClass().addClass('badge badge-success');
                    $('#status-sw').removeClass().addClass('badge badge-success');
                    $('#alert-notificaton').html('Automatic');
                    $('#emergency-button')
                        .addClass('btn-danger')
                        .removeClass('btn-success btn-secondary')
                        .html('Notaus')
                        .data('msg', '7#2#true')
                        .prop("disabled", false);
                    break;
            }
            break;
    }
}

function handleControlMessages(msgId, data) {
    switch(msgId) {
        case 2:
            for (let item of data) {
                console.debug("--->", item);
                if(item.trainId) {
                    setTrain(item.id, item.trainId);
                }
            }

        case 4:
            for (let item of data) {
                console.debug("--->", item);
                controlSwitch(item.id, item.switchStand);
            }
            break;

        case 6:
            for (let item of data) {
                trains[item.id] = item;
            }
            break;

        case 12:
            console.debug("--->", data);
            let svg = $("object")[0].contentDocument.documentElement;
            setTrain(data.toBlock, data.trainId);
            $(svg).find('#b' + data.fromBlock + " > tspan").text("");
            break;
    }
}

function controlSwitch(id, position) {
    let svg = $("object")[0].contentDocument.documentElement;
    $(svg).find("#switch_" + id + " > .bend1").hide();
    $(svg).find("#switch_" + id + " > .straight1").hide();
    $(svg).find("#switch_" + id + " > .bend2").hide();
    $(svg).find("#switch_" + id + " > .straight2").hide();

    switch(position) {
        case "STRAIGHT_1":
            $(svg).find("#switch_" + id + " > .straight1").show();
            break;

        case "BEND_1":
            $(svg).find("#switch_" + id + " > .bend1").show();
            break;

        case "STRAIGHT_2":
            $(svg).find("#switch_" + id + " > .straight2").show();
            break;

        case "BEND_2":
            $(svg).find("#switch_" + id + " > .bend2").show();
            break;
    }
}

function setTrain(blockId, trainId) {
    let svg = $("object")[0].contentDocument.documentElement;

    let train = trains[trainId];

    console.log(train);
    console.log(blockId);

    if(train.direction === "FORWARD") {
        $(svg).find('#b' + blockId + " > tspan").text(train.address + " >>");
    } else {
        $(svg).find('#b' + blockId + " > tspan").text(train.address + " <<");
    }
}

$(document).ready(function() {
    var ws = new WebSocket("ws://192.168.178.34:8080/display");
    ws.onmessage = function(event) {
        console.debug("WebSocket message received:", event.data);
        var d = JSON.parse(event.data);
        switch(d.groupId) {
            case 7:  // SYSTEM
                handleSystemMessages(d.messageId, d.data);
                break;

            case 10: // CONTROL
                handleControlMessages(d.messageId, d.data);
                break;
        }
    };
    ws.onopen = function(event) {
        ws.send('7#7#null');  // GET_HARDWARE_STATE
        ws.send('10#5#null'); // GET_TRAIN_LIST_REQ
        ws.send('10#1#null'); // GET_BLOCK_LIST_REQ
        ws.send('10#4#null'); // GET_SWITCH_STAND_LIST_REQ
    };

    $('#emergency-button').click(function(){
        ws.send($(this).data('msg'));
    });

    $('#direction-button').click(function(){
        ws.send('6#5#{"localId":' + $('#locoList').val() + ',"direction":"TOGGLE"}');
    });

    $('#locoSpeed').change(function(){
        ws.send('6#4#{"localId":' + $('#locoList').val() + ',"speed":' + $('#locoSpeed').val() + '}');
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
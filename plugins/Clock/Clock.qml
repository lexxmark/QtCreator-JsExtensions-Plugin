/*
* Copyright (C) 2010 Johannes Zellner <webmaster@nebulon.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

import QtQuick 2.1

Rectangle {
    id: clock
    //width : columnLayout.width
    width: 70
    height : columnLayout.height
    property color textColor : "steelblue"
    property int fontSize : 18
    property int fontSizeDate: 15
    property variant hours : "00"
    property variant minutes : "00"
    property variant seconds : "00"
    property string date: ""
    property variant shift : 0
    property bool showSeconds : true
    property bool showDate : false

    function timeChanged() {
        // To be fixed to fit locale
        var Month = new Array("Januar", "Februar", "M&auml;rz", "April", "Mai", "Juni",
                              "Juli", "August", "September", "Oktober", "November", "Dezember");
        var d = new Date;

        // hours
        var tmp = checkTime(shift ? d.getUTCHours() + Math.floor(clock.shift) : d.getHours())
        if (tmp != hours)
            hours = tmp

        // minutes
        tmp = checkTime(shift ? d.getUTCMinutes() + ((clock.shift % 1) * 60) : d.getMinutes())
        if (tmp != minutes)
            minutes = tmp

        // seconds
        seconds = checkTime(d.getUTCSeconds())

        // get Date
        date = d.getDate() + ". " + Month[d.getMonth()] + " " + d.getFullYear();
    }

    function checkTime(i) {
        return (i<10) ? "0"+i : i;
    }

    Timer {
        interval: 1000; running: true; repeat: true; triggeredOnStart: true
        onTriggered: clock.timeChanged()
    }

    Column {
        id: columnLayout

        Row {
            id : rowLayout
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: hoursText
                text: clock.hours
                color: clock.textColor
                font.pixelSize: clock.fontSize


                Behavior on text {
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation { target: hoursText; property: "opacity"; to: 0.1; duration: 200 }
                            NumberAnimation { target: hoursText; property: "y"; to: clock.height; duration: 300 }
                        }
                        PropertyAction {  }
                        NumberAnimation { target: hoursText; property: "y"; to: -clock.height; duration: 0 }
                        ParallelAnimation {
                            NumberAnimation { target: hoursText; property: "opacity"; to: 1; duration: 200 }
                            NumberAnimation { target: hoursText; property: "y"; to: 0; easing.type: Easing.OutBounce; duration: 400 }
                        }
                    }
                }
            }

            Text {
                text: ":"
                color: clock.textColor
                font.pixelSize: clock.fontSize
            }

            Text {
                id : minutesText
                text: clock.minutes
                color: clock.textColor
                font.pixelSize: clock.fontSize

                Behavior on text {
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation { target: minutesText; property: "opacity"; to: 0.1; duration: 200 }
                            NumberAnimation { target: minutesText; property: "y"; to: clock.height; duration: 300 }
                        }
                        PropertyAction {  }
                        NumberAnimation { target: minutesText; property: "y"; to: -clock.height; duration: 0 }
                        ParallelAnimation {
                            NumberAnimation { target: minutesText; property: "opacity"; to: 1; duration: 200 }
                            NumberAnimation { target: minutesText; property: "y"; to: 0; easing.type: Easing.OutBounce; duration: 400 }
                        }
                    }
                }
            }

            Text {
                text: ":"
                color: clock.textColor
                font.pixelSize: clock.fontSize
                visible : clock.showSeconds
            }

            Text {
                id : secondsText
                text: clock.seconds
                color: clock.textColor
                font.pixelSize: clock.fontSize
                visible : clock.showSeconds

                Behavior on text {
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation { target: secondsText; property: "opacity"; to: 0.1; duration: 200 }
                            NumberAnimation { target: secondsText; property: "y"; to: clock.height; duration: 300 }
                        }
                        PropertyAction {  }
                        NumberAnimation { target: secondsText; property: "y"; to: -clock.height; duration: 0 }
                        ParallelAnimation {
                            NumberAnimation { target: secondsText; property: "opacity"; to: 1; duration: 200 }
                            NumberAnimation { target: secondsText; property: "y"; to: 0; easing.type: Easing.OutBounce; duration: 400 }
                        }
                    }
                }
            }
        }

        Text {
            id : dateText
            text: clock.date
            color: parent.parent.textColor
            font.pixelSize: clock.fontSizeDate
            visible : clock.showDate
        }
    }
}

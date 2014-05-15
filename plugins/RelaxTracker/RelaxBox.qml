import QtQuick 2.1
import QtGraphicalEffects 1.0

Rectangle {
    id: relaxBox
    width: 70
    height:30

    // values in seconds
    property int workDuration: 10
    property int relaxDuration: 5
    property int timerPeriod: 1

    property date lastAlarmTime: new Date

    function restore() {
        relaxBox.color = "green";
        label.opacity = 1;
        label.text = "Work";
        relaxBox.lastAlarmTime = new Date;
        timer.start();
    }

    Component.onCompleted: restore()

    Text {
        id: label
        color: "white"
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        SequentialAnimation on opacity {
            id: flashing
            running: false

            SequentialAnimation {
                loops: relaxBox.relaxDuration

                PropertyAnimation {
                    duration: 500
                    to: 0
                }
                PropertyAnimation {
                    duration: 500
                    to: 1
                }
            }

            ScriptAction {
                script: relaxBox.restore();
            }
        }
    }

    function timeChanged() {
        var now = new Date;
        var durationSec = (now.getTime() - relaxBox.lastAlarmTime.getTime())/1000;

        if (durationSec > relaxBox.workDuration)
        {
            relaxBox.color = "red";
            //core.showWarningWithOptions("Attention", "It's time to relax.", "", "", "", null);
            label.text = "Break";
            messageManager.write("It's time to relax!", 512);
            timer.stop();
            flashing.start();
        }
    }

    Timer {
        id: timer
        interval: 1000*relaxBox.timerPeriod;
        running: true;
        repeat: true;
        triggeredOnStart: true
        onTriggered: relaxBox.timeChanged()
    }
}

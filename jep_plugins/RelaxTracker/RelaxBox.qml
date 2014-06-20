import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
//import "RelaxSettingsDlg.qml"

Rectangle {
    id: relaxBox
    width: 70
    height: 30

    // values in seconds
    property int workDuration: 10*60
    property int relaxDuration: 2*60
    property int timerPeriod: 1

    property date lastAlarmTime: new Date

    function restore() {
        relaxBox.color = "green";
        label.opacity = 1;
        label.text = "Work";
        relaxBox.lastAlarmTime = new Date;
        timer.start();
    }

    function timeChanged() {
        var now = new Date;
        var durationSec = (now.getTime() - relaxBox.lastAlarmTime.getTime())/1000;

        if (durationSec > relaxBox.workDuration) {
            relaxBox.color = "red";
            //core.showWarningWithOptions("Attention", "It's time to relax.", "", "", "", null);
            label.text = "Break";
            messageManager.write("It's time to relax!", 512);
            timer.stop();
            flashing.start();
        }
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

    Timer {
        id: timer
        interval: 1000*relaxBox.timerPeriod
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: relaxBox.timeChanged()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: contextMenu.popup()
    }

    Menu {
        id: contextMenu
        title: "ContextMenu"
        MenuItem {
            text: "Settings"
            onTriggered: showSettings()
        }
    }

    RelaxSettingsDlg {
        id: settingsDlg

        onAccepted: {
            relaxBox.workDuration = settingsDlg.workDuration*60;
            relaxBox.relaxDuration = settingsDlg.relaxDuration*60;
        }
    }

    function showSettings() {
        settingsDlg.showModal(relaxBox.workDuration/60, relaxBox.relaxDuration/60);
    }
}

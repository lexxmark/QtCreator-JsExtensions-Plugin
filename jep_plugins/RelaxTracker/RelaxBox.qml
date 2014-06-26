import QtQuick 2.1
import QtQuick.Controls 1.1
import ".."

Rectangle {
    id: root

    width: 70
    height: 30

    JepSettings {
        id: settings
        settingsPath: "RT.settings"

        // values in minutes
        property int workDuration: 10
        property int relaxDuration: 2
        property int timerPeriod: 1
    }

    property date lastAlarmTime: new Date

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
                loops: settings.relaxDuration*60

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
                script: root.restore();
            }
        }
    }

    Timer {
        id: timer
        interval: 1000*60*settings.timerPeriod
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: root.timeChanged()
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
            text: "Settings..."
            onTriggered: showSettings()
        }
    }

    RelaxSettingsDlg {
        id: settingsDlg

        onAccepted: {
            settings.workDuration = settingsDlg.workDuration;
            settings.relaxDuration = settingsDlg.relaxDuration;
        }
    }

    Component.onCompleted: {
        settings.load();
        restore();
    }

    function restore() {
        root.color = "green";
        label.opacity = 1;
        label.text = "Work";
        root.lastAlarmTime = new Date;
        timer.start();
    }

    function timeChanged() {
        var now = new Date;
        var durationSec = (now.getTime() - root.lastAlarmTime.getTime())/1000;

        if (durationSec > (settings.workDuration*60)) {
            root.color = "red";
            label.text = "Break";
            messageManager.write("It's time to relax!", 512);
            timer.stop();
            flashing.start();
        }
    }

    function showSettings() {
        settingsDlg.showModal(settings.workDuration, settings.relaxDuration);
    }
}

import QtQuick 2.1
import QtQuick.Controls 1.1

Rectangle {
    id: relaxBox
    width: 70
    height: 30

    // values in minutes
    property int workDuration: 10
    property int relaxDuration: 2
    property int timerPeriod: 1

    property date lastAlarmTime: new Date

    Component.onCompleted: {
        loadSettings();
        restore();

        // save settings at plugin shutdown
        // Component.onDestruction cannot be used because
        // jepAPI is null at this moment
        jepAPI.aboutToShutdown.connect(saveSettings);
    }

    function loadSettings() {
        var content = jepAPI.loadFile("RT.settings");
        if (content !== '') {
            var params = JSON.parse(content);
            if ('workDuration' in params)
                relaxBox.workDuration = params.workDuration;
            if ('relaxDuration' in params)
                relaxBox.relaxDuration = params.relaxDuration;
        }
    }

    function saveSettings() {
        var params = {
            'workDuration': relaxBox.workDuration,
            'relaxDuration': relaxBox.relaxDuration
        };
        jepAPI.saveFile("RT.settings", JSON.stringify(params));
    }

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

        if (durationSec > (relaxBox.workDuration*60)) {
            relaxBox.color = "red";
            //core.showWarningWithOptions("Attention", "It's time to relax.", "", "", "", null);
            label.text = "Break";
            messageManager.write("It's time to relax!", 512);
            timer.stop();
            flashing.start();
        }
    }

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
                loops: relaxBox.relaxDuration*60

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
        interval: 1000*60*relaxBox.timerPeriod
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
            relaxBox.workDuration = settingsDlg.workDuration;
            relaxBox.relaxDuration = settingsDlg.relaxDuration;
        }
    }

    function showSettings() {
        settingsDlg.showModal(relaxBox.workDuration, relaxBox.relaxDuration);
    }
}

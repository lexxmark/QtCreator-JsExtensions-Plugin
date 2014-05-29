import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Rectangle {
    id: relaxBox
    width: 70
    height: 30

    // values in seconds
    property int workDuration: 10*60
    property int relaxDuration: 2*60
    property int timerPeriod: 1

    property date lastAlarmTime: new Date

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

    Window {
        id: settingsDlg
        modality: Qt.WindowModal
        visible: false
        title: "Relax Tracker settings"
        minimumWidth: rowsLayout.implicitWidth+20
        minimumHeight: gridLayout.implicitHeight+20

        RowLayout {
            id: rowsLayout
            anchors.fill: parent
            anchors.margins: 10
            //anchors.verticalCenter: parent.verticalCenter

            GridLayout {
                id: gridLayout
                anchors.leftMargin: 10
                columns: 2
                Text {
                    text: "WorkDuration"
                }
                SpinBox {
                    id: wd
                    implicitWidth: 100
                    suffix: "min"
                    value: relaxBox.workDuration/60
                }

                Text {
                    text: "RelaxDuration"
                }
                SpinBox {
                    id: rd
                    implicitWidth: 100
                    suffix: "min"
                    value: relaxBox.relaxDuration/60
                }
            }
            ColumnLayout {
                anchors.right: parent.right

                Button {
                    text: "OK"
                    onClicked: settingsDlg.apply()
                }
                Button {
                    text: "Cancel"
                    onClicked: settingsDlg.close()
                }
            }
        }

        function apply() {
            settingsDlg.close();
            jepAPI.debug(wd.value);
            jepAPI.debug(rd.value);
        }
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

        if (durationSec > relaxBox.workDuration) {
            relaxBox.color = "red";
            //core.showWarningWithOptions("Attention", "It's time to relax.", "", "", "", null);
            label.text = "Break";
            messageManager.write("It's time to relax!", 512);
            timer.stop();
            flashing.start();
        }
    }

    function showSettings() {
        settingsDlg.show();
    }
}

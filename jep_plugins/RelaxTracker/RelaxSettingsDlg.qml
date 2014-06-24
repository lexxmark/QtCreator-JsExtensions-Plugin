import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Window {
    id: root

    property int workDuration: wd.value
    property int relaxDuration: rd.value

    signal accepted

    modality: Qt.WindowModal
    visible: false
    title: "Relax Tracker settings"
    minimumWidth: rowsLayout.implicitWidth+20
    minimumHeight: gridLayout.implicitHeight+20

    function showModal(workDuration, relaxDuration)
    {
        wd.value = workDuration;
        rd.value = relaxDuration;
        root.show();
    }

    RowLayout {
        id: rowsLayout
        anchors.fill: parent
        anchors.margins: 10

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
                minimumValue: 1
                maximumValue: 60
            }

            Text {
                text: "RelaxDuration"
            }
            SpinBox {
                id: rd
                implicitWidth: 100
                suffix: "min"
                minimumValue: 1
                maximumValue: 60
            }
        }
        ColumnLayout {
            anchors.right: parent.right

            Button {
                text: "OK"
                onClicked: {
                    root.accepted();
                    root.close();
                }
            }
            Button {
                text: "Cancel"
                onClicked: {
                    root.close();
                }
            }
        }
    }
}

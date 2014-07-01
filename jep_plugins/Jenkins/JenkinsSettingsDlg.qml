import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import "../JepUtils.js" as JepUtils

Window {
    id: root

    property alias server: server.text

    signal accepted(string server, string view)

    modality: Qt.WindowModal
    visible: false
    title: "Jenkins settings"
    minimumWidth: rowsLayout.implicitWidth+20
    minimumHeight: rowsLayout.implicitHeight+20

    RowLayout {
        id: rowsLayout
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {

            RowLayout {
                id: sm

                TextField {
                    id: server
                    placeholderText: "Enter Jenking server"
                    Layout.fillWidth: true
                    implicitWidth: 200
                }

                Button {
                    text: "Find views"
                    enabled: server.text.length !== 0
                    onClicked: fillViewsList()
                }
            }

            Item {
                id: views

                //width: sm.width
                Layout.fillWidth: true
                Layout.fillHeight: true
                implicitHeight: 200

                function error(errorText) {
                    listParent.opacity = 0;
                    list.currentIndex = -1;
                    errorLabel.text = (errorText.length !== 0) ? errorText : "Unknown error";
                    errorLabel.opacity = 1;
                }

                function success() {
                    errorLabel.opacity = 0;
                    listParent.opacity = 1;
                }

                ScrollView {
                    id: listParent
                    anchors.fill: parent
                    opacity: 0

                    ListView {
                        id: list
                        focus: true

                        model: ListModel { id: model }

                        delegate: viewDelegate
                        Component {
                            id: viewDelegate

                            Text {
                                id: viewName
                                text: model.name;
                                width: list.width

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: list.currentIndex = index
                                }
                            }
                        }

                        highlight: viewHighlight
                        Component {
                            id: viewHighlight

                            Rectangle {
                                width: list.width
                                color: "lightGreen"
                            }
                        }
                    }
                }

                Text {
                    id: errorLabel
                    color: "lightgray"
                    font.bold: true
                    opacity: (text.length === 0) ? 0 : 1
                }

            }

        }

        ColumnLayout {
            anchors.top: parent.top
            anchors.right: parent.right

            Button {
                text: "OK"
                enabled: list.currentItem !== null
                onClicked: {
                    var item = model.get(list.currentIndex);
                    root.accepted(root.server, item.name);
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

    function fillViewsList() {
        var url = "%1/api/json?pretty=true".arg(root.server);
        JepUtils.loadJSON(url, onLoaded);
    }

    function onLoaded(obj, error) {
        if (obj === null) {
            views.error(error);
            return;
        }

        if (obj.views.length === 0) {
            views.error("No views available");
            return;
        }

        model.clear();
        model.append(obj.views);
        views.success();
    }
}

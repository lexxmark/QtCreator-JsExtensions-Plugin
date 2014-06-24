import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Window {
    id: root

    property int cityId: 0

    signal accepted(int cityId)

    modality: Qt.WindowModal
    visible: false
    title: "Select City"
    minimumWidth: rowsLayout.implicitWidth+20
    minimumHeight: rowsLayout.implicitHeight+20

    RowLayout {
        id: rowsLayout
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {
            anchors.top: parent.top

            RowLayout {

                TextField {
                    id: cityName
                    placeholderText: "Enter City Name"
                    Layout.fillWidth: true
                    implicitWidth: 200
                }

                Button {
                    text: "Find"
                    enabled: cityName.text.length !== 0
                    onClicked: fillCitiesList()
                }
            }

            Item {
                id: cities

                width: parent.width
                Layout.fillHeight: true
                implicitHeight: 20

                function error(errorText) {
                    list.opacity = 0;
                    list.currentIndex = -1;
                    errorLabel.text = (errorText.length !== 0) ? errorText : "Unknown error";
                    errorLabel.opacity = 1;
                }

                function success() {
                    errorLabel.opacity = 0;
                    list.opacity = 1;
                }

                ScrollView {
                    ListView {
                        id: list

                        anchors.fill: parent
                        opacity: 0
                        focus: true

                        model: ListModel { id: model }

                        delegate: cityDelegate
                        Component {
                            id: cityDelegate

                            Text {
                                id: cityName
                                text: "%1 (%2)".arg(name).arg(sys.country);

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: list.currentIndex = index
                                }
                            }
                        }

                        highlight: cityHighlight
                        Component {
                            id: cityHighlight

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
                    opacity: 0
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
                    root.accepted(item.id);
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

    function fillCitiesList() {
        var doc = new XMLHttpRequest();
        doc.onreadystatechange = function() {
            var s = doc.readyState;
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status === 200)
                    onCitiesListLoaded(doc);
                else
                    cities.error(doc.statusText);
            }
        }

        var url = "http://api.openweathermap.org/data/2.5/find?q=%1&type=like&APPID=79e8f327892ff196b77b400988291a2a".arg(cityName.text);
        doc.open("get", url);
        doc.setRequestHeader("Content-Encoding", "UTF-8");
        doc.send();
    }

    function onCitiesListLoaded(doc) {
        var responseText = doc.responseText;
        if (responseText.length === 0) {
            cities.error("Loading Failed");
            return;
        }

        var rObj = JSON.parse(responseText);

        if (rObj.cod !== "200") {
            cities.error("%1-'%2'".arg(rObj.cod).arg(rObj.message));
            return;
        }

        if (rObj.count === 0) {
            cities.error("Search result is empty");
            return;
        }

        model.clear();
        model.append(rObj.list);
        cities.success();
    }
}

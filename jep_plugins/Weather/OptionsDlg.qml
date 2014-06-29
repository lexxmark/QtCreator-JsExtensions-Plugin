import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Window {
    id: root

    signal accepted(bool metric, string lang)

    modality: Qt.WindowModal
    visible: false
    title: "Weather Options"
    minimumWidth: rowsLayout.implicitWidth+20
    minimumHeight: rowsLayout.implicitHeight+20

    RowLayout {
        id: rowsLayout
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {
            anchors.top: parent.top

            Column {
                spacing: 2

                ExclusiveGroup { id: group }

                RadioButton {
                    id: celsiusBttn
                    text: "Celsius"
                    exclusiveGroup: group
                }
                RadioButton {
                    id: fahrenheitBttn
                    text: "Fahrenheit"
                    exclusiveGroup: group
                }
            }

            ComboBox {
                id: langCombo
                textRole: "name"

                model: ListModel {
                    id: langModel
                    ListElement { name: "English"; lang: "en" }
                    ListElement { name: "Russian"; lang: "ru" }
                    ListElement { name: "Italian"; lang: "it" }
                    ListElement { name: "Spanish"; lang: "sp" }
                    ListElement { name: "Ukrainian"; lang: "ua" }
                    ListElement { name: "German"; lang: "de" }
                    ListElement { name: "Portuguese"; lang: "pt" }
                    ListElement { name: "Romanian"; lang: "ro" }
                    ListElement { name: "Polish"; lang: "pl" }
                    ListElement { name: "Finnish"; lang: "fi" }
                    ListElement { name: "Dutch"; lang: "nl" }
                    ListElement { name: "French"; lang: "fr" }
                    ListElement { name: "Bulgarian"; lang: "bg" }
                    ListElement { name: "Swedish"; lang: "se" }
                    ListElement { name: "Chinese Traditional"; lang: "zh_tw" }
                    ListElement { name: "Chinese Simplified"; lang: "zh_cn" }
                    ListElement { name: "Turkish"; lang: "tr" }
                }
            }
        }

        ColumnLayout {
            anchors.top: parent.top
            anchors.right: parent.right

            Button {
                text: "OK"
                onClicked: {
                    root.accepted(celsiusBttn.checked, langModel.get(langCombo.currentIndex).lang);
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

    function showModal(celsius, lang) {
        if (celsius)
            celsiusBttn.checked = true;
        else
            fahrenheitBttn.checked = true;

        for (var i = 0, n = langModel.count; i < n; ++i) {
            if (lang === langModel.get(i).lang) {
                langCombo.currentIndex = i;
                break;
            }
        }

        root.show();
    }
}

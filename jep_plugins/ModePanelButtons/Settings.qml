import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import ".."

JepDialog {
    id: root

    width: rowsLayout.implicitWidth+20
    height: rowsLayout.implicitHeight+20

    JepSettings {
        id: settings
        settingsPath: "MPB.settings"
        autoSave: false

        property var actions: []
        property bool showIcons: false
    }

    RowLayout {
        id: rowsLayout
        anchors.fill: parent
        anchors.margins: 10

        ScrollView {
            //implicitWidth: 300
            Layout.fillWidth: true
            //height: parent.height
            Layout.fillHeight: true

            ListView {
                id: cmdList
                anchors.fill: parent
                focus: true

                model: ListModel {
                    id: cmdModel
                }

                delegate: CheckBox {
                    checked: use
                    text: name

                    onCheckedChanged: {
                        cmdModel.get(index)["use"] = checked;
                    }
                }
            }
        }

        ColumnLayout {
            anchors.top: parent.top
            anchors.right: parent.right

            Button {
                text: "OK"
                onClicked: {
                    root.accept();
                }
            }
            Button {
                text: "Cancel"
                onClicked: {
                    root.reject();
                }
            }
        }
    }

    Component.onCompleted: {
        settings.load();
        if (!(settings.actions instanceof Array)) {
            jepAPI.debug("<actions> attribute is not array");
            return;
        }

        var commands = actionManager.commands();
        var cmds = [];
        commands.map(function (command) {
            var id = command.id();
            var item = {
                name: id,
                use: (settings.actions.indexOf(id) != -1)
            };
            cmds.push(item);
        });

        function sortByKey(array, key) {
            return array.sort(function(a, b) {
                var x = a[key]; var y = b[key];
                return ((x < y) ? -1 : ((x > y) ? 1 : 0));
            });
        }
        sortByKey(cmds, "name");

        cmdModel.clear();
        cmdModel.append(cmds);
    }

    Component.onDestruction: {
        if (status === sAccepted) {
            var newActions = [];
            for (var i = 0, n = cmdModel.count; i < n; ++i) {
                var cmd = cmdModel.get(i);
                if (cmd["use"])
                    newActions.push(cmd["name"]);
            }
            settings.actions = newActions;
            settings.save();
        }
    }
}


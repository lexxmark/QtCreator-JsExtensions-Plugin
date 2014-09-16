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

        ColumnLayout {

            RowLayout {
                id: sm

                TextField {
                    id: filter
                    placeholderText: "Enter text to filter"
                    Layout.fillWidth: true
                    implicitWidth: 200
                }

                Button {
                    text: "Filter"
                    //enabled: filter.text.length !== 0
                    onClicked: {
                        root.fillSettingsActions();
                        root.fillCommands();
                    }
                }
            }

            Item {
                id: views

                //width: sm.width
                Layout.fillWidth: true
                Layout.fillHeight: true
                implicitHeight: 200

                ScrollView {
                    //implicitWidth: 300
                    //Layout.fillWidth: true
                    //height: parent.height
                    //Layout.fillHeight: true
                    anchors.fill: parent

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

        // remove duplicates
        settings.actions = settings.actions.sort().filter(function(item, pos) {
            return !pos || item !== settings.actions[pos - 1];
        });

        root.fillCommands();
    }

    Component.onDestruction: {
        if (status === sAccepted) {
            fillSettingsActions();
        }
    }

    function fillCommands() {
        var commands = actionManager.commands();
        var cmds = [];
        var filterText = filter.text.toLowerCase();
        var useFilter = filterText.length !== 0;
        jepAPI.debug(filterText);
        commands.map(function (command) {
            var id = command.id();

            if (useFilter && (id.toLowerCase().search(filterText) === -1))
                return;

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

    function fillSettingsActions() {
        for (var i = 0, n = cmdModel.count; i < n; ++i) {
            var cmd = cmdModel.get(i);
            var actionIndex = settings.actions.indexOf(cmd["name"]);
            var hasCmd = (actionIndex !== -1);
            if (cmd["use"] && !hasCmd)
                settings.actions.push(cmd["name"]);
            else if (!cmd["use"] && hasCmd)
                settings.actions.splice(actionIndex, 1);
        }
        settings.save();
    }
}

import QtQuick 2.1
import QtQuick.Controls 1.1
import ".."
import "../JepUtils.js" as JepUtils

Rectangle {
    id: root

    //width: 70
    height: (jobsList.headerItem.implicitHeight+10)*settings.lastJobs + jobsList.headerItem.implicitHeight

    color: "gray"

    JepSettings {
        id: settings
        settingsPath: "Jenkins.settings"

        property string server: "http://build.kde.org"
        property string imagesPath: "http://build.kde.org/static/6cf0c7e2/images/16x16/"
        property string view: "Unstable" //"Plasma%20Next%20+%20Apps"
        property int lastJobs: 3
        property int refreshPeriod: 10 // min
    }

    ListView {
        id: jobsList
        anchors.fill: parent

        header: Component {
            Text {
                id: jobsHeader
                text: "None"

                property string viewName: ""

                onLinkActivated: {
                    settingsDlg.server = settings.server;
                    settingsDlg.show();
                }

                MouseArea {
                    acceptedButtons: Qt.NoButton
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: jobsHeader.text = "<a href=\"%1\">%1</a>".arg(jobsHeader.viewName)
                    onExited: jobsHeader.text = jobsHeader.viewName
                }
            }
        }

        model: ListModel { id: jobsModel }

        delegate: jobsDelegate
        Component {
            id: jobsDelegate
            Item {
                height: jobName.height+10

                Image {
                    id: jobImage
                    anchors.verticalCenter: parent.verticalCenter
                    source: "%1%2.png".arg(settings.imagesPath).arg(model.color.split("_")[0])
                }

                Text {
                    id: jobName
                    anchors.left: jobImage.right
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter

                    text: model.name

                    onLinkActivated: Qt.openUrlExternally(link)

                    MouseArea {
                        acceptedButtons: Qt.NoButton
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: jobName.text = "<a href=\"%1\">%2</a>".arg(model.url).arg(model.name)
                        onExited: jobName.text = model.name
                    }
                }
            }
        }

        function setHeader(text) {
            jobsList.headerItem.text = text;
            jobsList.headerItem.viewName = text;
        }
    }

    Component.onCompleted: {
        settings.load();
        startLoadInfo();
    }

    function startLoadInfo() {

        jobsList.setHeader("Loading...");
        jobsModel.clear();

        var url = "%1/view/%2/api/json?pretty=true".arg(settings.server).arg(settings.view);
        JepUtils.loadJSON(url, onLoaded);
    }

    function onLoaded(obj, error) {
        if (obj === null) {
            jobsList.setHeader(error);
            return;
        }

        if (obj.jobs.length === 0) {
            jobsList.setHeader("No jobs available");
            return;
        }

        jobsList.setHeader(obj.name);

        var jobs = obj.jobs.slice(0, settings.lastJobs);
        jobsModel.append(jobs);
    }

    JenkinsSettingsDlg {
        id: settingsDlg
        onAccepted: {
            settings.server = server;
            settings.view = view;
            startLoadInfo();
        }
    }

    Timer {
        id: time
        running: true
        repeat: true
        interval: settings.refreshPeriod*60*1000

        onTriggered: startLoadInfo()
    }
}

import QtQuick 2.1
import QtQuick.Controls 1.1
import ".."
import "../JepUtils.js" as JepUtils

Rectangle {
    id: root

    //width: 70
    height: (jobsList.headerItem.implicitHeight+10)*settings.lastJobs + jobsList.headerItem.implicitHeight

    color: "gray"

    Text {
        id: message
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        text: "Loading..."
    }

    ListView {
        id: jobsList
        anchors.fill: parent
        opacity: 0

        header: Component {
            Text {
                id: jobsHeader
                text: "None"
            }
        }

        model: ListModel { id: jobsModel }

        delegate: jobsDelegate
        Component {
            id: jobsDelegate
            Item {
                height: jobName.height+10

                Rectangle {
                    id: jobStatus
                    height: jobName.height
                    width: jobName.height
                    radius: jobName.height/2
                    anchors.verticalCenter: parent.verticalCenter

                    color: Qt.darker(jobsModel.get(index).color, 2)
                }

                Text {
                    id: jobName
                    anchors.left: jobStatus.right
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter

                    text: name
                }
            }
        }
    }

    //JepSettings {
    QtObject {
        id: settings
        //settingsPath: "Jenkins.settings"

        // /api/json?pretty=true

        property string server: "http://build.kde.org"
        property string view: "Plasma%20Next%20+%20Apps"
        property int lastJobs: 3
        property int refreshPeriod: 10 // min
    }

    Component.onCompleted: startLoadInfo()

    function startLoadInfo() {
        message.opacity = 1;
        var url = "%1/view/%2/api/json?pretty=true".arg(settings.server).arg(settings.view);
        JepUtils.loadJSON(url, onLoaded);
    }

    function onLoaded(obj, error) {
        if (obj === null) {
            message.text = error;
            return;
        }

        if (obj.jobs.length === 0) {
            message.text = "No jobs available";
            return;
        }

        jobsList.headerItem.text = obj.name;
        jobsModel.clear();
        var jobs = obj.jobs.slice(0, settings.lastJobs);
        jobsModel.append(jobs);

        message.opacity = 0;
        jobsList.opacity = 1;
    }
}

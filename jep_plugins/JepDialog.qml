import QtQuick 2.1

Item {
    id: root

    property int status: 0

    readonly property int sAccepted: 1
    readonly property int sRejected: 0

    signal close(int status)

    function done(status) {
        root.status = status;
        close(status);
    }

    function accept() {
        done(sAccepted);
    }

    function reject() {
        done(sRejected);
    }
}

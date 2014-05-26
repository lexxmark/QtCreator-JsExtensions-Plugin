var pluginPriority = -1;
var pluginTrace = true;

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {
    var w = jepAPI.createQuickView("RelaxBox.qml", modeManager);
    modeManager.addWidget(w);
    modeManager.addWidget(createSpacer(10));
}

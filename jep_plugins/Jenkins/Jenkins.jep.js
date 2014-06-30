var pluginPriority = -2;
var pluginDescription = "Shows Jenkins status";

function initialize() {
    var w = jepAPI.createQuickView("Jenkins.qml", modeManager);
    modeManager.addWidget(w);
}

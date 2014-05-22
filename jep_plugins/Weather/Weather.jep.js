var pluginOrder = 2;
//var pluginDisable = true;

function initialize() {
    var w = jepAPI.createQuickView("Weather.qml", modeManager);
    modeManager.addWidget(w);
}

//var pluginTrace = true;
var pluginOrder = 2;
//var pluginDisable = true;

function createWeather() {
    return jepAPI.createQuickView("Weather.qml", null);
}

function initialize() {
    var w = jepAPI.createQuickView("Weather.qml", modeManager);
    modeManager.addWidget(w);
    jepAPI.registerNavigationWidgetFactory(createWeather, "Weather", 0, "", "");
}

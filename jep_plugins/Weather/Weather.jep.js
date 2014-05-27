//var pluginTrace = true;
var pluginPriority = 2;
var pluginDescription = "Shows current weather";

function createWeather() {
    return jepAPI.createQuickView("Weather.qml", null);
}

function initialize() {
    var w = jepAPI.createQuickView("Weather.qml", modeManager);
    modeManager.addWidget(w);
    jepAPI.registerNavigationWidgetFactory(createWeather, "Weather", 0, "", "");
}

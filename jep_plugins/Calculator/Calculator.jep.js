//var pluginTrace = true;
var pluginPriority = 2;
var pluginDescription = "Calculator widget on navigation panel";

function createCalculator() {
    return jepAPI.createQuickView("Calculator.qml", null);
}

function initialize() {
    //var w = jepAPI.createQuickView("Weather.qml", modeManager);
    //modeManager.addWidget(w);
    jepAPI.registerNavigationWidgetFactory(createCalculator, "Calcualtor", 0, "", "");
}

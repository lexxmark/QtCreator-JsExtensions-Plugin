var pluginPriority = 1;
var pluginDescription = "Shows current time in digital clock";

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {
    jepAPI.registerNavigationWidgetFactory("Clocks.qml", "Clock", 0, "", "");
}

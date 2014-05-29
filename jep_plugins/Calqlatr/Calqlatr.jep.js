var pluginPriority = 1;
var pluginDescription = "Adds simple calculator to navigation panel";

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {
    jepAPI.registerNavigationWidgetFactory("Calqlatr.qml", "SimpleCalculator", 0, "", "");
}

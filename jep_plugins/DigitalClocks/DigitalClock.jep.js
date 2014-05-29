var pluginPriority = 1;
var pluginDescription = "Shows current time in digital clock";

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {

    var w = jepAPI.createQuickView("DigitalClock.qml", modeManager);
    modeManager.addWidget(w);

    //modeManager.addWidget(createSpacer(10));

    jepAPI.registerNavigationWidgetFactory("DigitalClock.qml", "DigitalClock", 0, "", "");

    //jepAPI.dumpPluginManagerObjects();
}

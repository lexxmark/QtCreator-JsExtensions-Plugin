var pluginOrder = 1;
//var pluginDisable = true;

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {

    //var w = jepAPI.createQuickView("Clock.qml", modeManager);
    //modeManager.addWidget(w);

    //modeManager.addWidget(createSpacer(10));

    jepAPI.registerNavigationWidgetFactory("Clock.qml", "Clocks", 0, "", "");
}

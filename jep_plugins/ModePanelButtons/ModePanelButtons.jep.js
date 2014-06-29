jepAPI.include("../JepUtils.js");

var pluginPriority = 1;
var pluginDescription = "Adds buttons to mode panel";

function initialize() {

    // load settings
    var content = jepAPI.loadFile("MPB.settings");
    if (!content) {
        jepAPI.debug("Cannot load settings");
        return;
    }
    // parse settings
    var settings = JSON.parse(content);
    if (!(settings.actions instanceof Array)) {
        jepAPI.debug("<actions> attribute is not array");
        return;
    }

    // add action buttons to mode panel
    settings.actions.map(function (actionName) {
        var bttn = createActionToolButton(actionName);
        if (bttn)
            modeManager.addWidget(bttn);
    });
}

function settings(parent) {
    var w = jepAPI.quickDialogExec("Settings.qml", parent);
}

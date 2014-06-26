jepAPI.include("../JepUtils.js");

var pluginDescription = "Adds 'Save All' button to mode panel";

function initialize() {
    modeManager.addWidget(createActionToolButton("QtCreator.SaveAll"));
    modeManager.addWidget(createModeSpacer(10));

    //jepAPI.dumpCommands();

    jepAPI.debug("Success initialize");
}

function extensionsInitialized() {
    jepAPI.debug("Success extensionsInitialized");
}

function aboutToShutdown() {
    jepAPI.debug("Success aboutToShutdown");
}

var pluginDescription = "Adds 'Save All' button to mode panel";

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function createSaveAllButton() {

    var bttn = jepAPI.createQObject("QToolButton", modeManager);
    bttn.toolButtonStyle = 2;
    bttn.autoRaise = true;
    bttn.styleSheet = "QToolButton {color: white; }";
    // disable button minimum width
    bttn.sizePolicy = jepAPI.sizePolicy(13, 0, 1);

    var cmd = actionManager.command("QtCreator.SaveAll");
    bttn.setDefaultAction(cmd.action());

    return bttn;
}

function initialize() {

    modeManager.addWidget(createSaveAllButton());
    modeManager.addWidget(createSpacer(10));

    //jepAPI.dumpCommands();

    jepAPI.debug("Success initialize");
}

function extensionsInitialized() {
    jepAPI.debug("Success extensionsInitialized");
}

function aboutToShutdown() {
    jepAPI.debug("Success aboutToShutdown");
}

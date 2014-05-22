var pluginTrace = true;

function saveAllAction() {
    var docs = editorManager.documents();
    for (var i = 0; i < docs.length; ++i)
    {
        var doc = docs[i];
        if (doc.isModified())
        {
            doc.save("", false);
        }
    }
}

function createSaveAllButton() {
    var bttn = jepAPI.createQObject("QPushButton", modeManager);
    bttn.flat = true;
    bttn.text = "Save All";
    bttn.focusPolicy = 0;
    bttn.styleSheet = "QPushButton {color: white; }";
    // disable button minimum width
    bttn.sizePolicy = jepAPI.sizePolicy(13, 0, 1);

    bttn.clicked.connect(saveAllAction);

    return bttn;
}

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {

    modeManager.addWidget(createSaveAllButton());
    modeManager.addWidget(createSpacer(10));

    jepAPI.debug("Success initialize");
}

function extensionsInitialized() {
    jepAPI.debug("Success extensionsInitialized");
}

function aboutToShutdown() {
    jepAPI.debug("Success aboutToShutdown");
}

var pluginDescription = "Adds \"Close All\" button to mode panel";

function closeAllAction() {
    editorManager.closeAllEditors(true);
}

function createCloseAllButton() {
    var bttn = jepAPI.createQObject("QPushButton", null);
    bttn.flat = true;
    bttn.text = "Close All";
    bttn.toolTip = "Close all opened documents";
    bttn.focusPolicy = 0;
    bttn.styleSheet = "QPushButton {color: white; }";
    // disable button minimum width
    bttn.sizePolicy = jepAPI.sizePolicy(13, 0, 1);

    bttn.clicked.connect(closeAllAction);

    return bttn;
}

function createSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", modeManager);
    spacer.minimumHeight = space;
    return spacer;
}

function initialize() {
    modeManager.addWidget(createCloseAllButton());
    modeManager.addWidget(createSpacer(10));
}

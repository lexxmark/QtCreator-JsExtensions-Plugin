var pluginEnable = false;
var pluginDescription = "Adds \"Close All\" menu item to Tools submenu";

function closeAllAction() {
	editorManager.closeAllEditors(true);

    // uncomment to see message box
    /*
    var msgBox = jepAPI.createQObject("QMessageBox", core.dialogParent());

    msgBox.modal = true;
    msgBox.windowTitle = "Window title";
    msgBox.text = "Text";
    msgBox.detailedText = "Detailed text";
    msgBox.exec();
    */

    core.showWarningWithOptions("Information", "All editors were closed", "", "", "", null);
}

function installCloseAllToolMenu() {
    var action = jepAPI.createQObject("QAction", actionManager);
    action.text = "Close All";
/*    var cmd = actionManager.registerAction(action, "JsExtentionsPlugin.Action", "Global Context", false);
/*	cmd.setDefaultKeySequence("Ctrl+Alt+Meta+C");
	action.triggered.connect(closeAllAction);
    var menu = actionManager.createMenu("JsExtentionsPlugin.Menu");
    menu.menu().title = "CloseAllMenu";
	menu.addAction(cmd, "");
	actionManager.actionContainer("QtCreator.Menu.Tools").addMenu1(menu, "");
    */
}

function initialize() {
    installCloseAllToolMenu();
}

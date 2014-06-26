
function createModeSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", null);
    spacer.minimumHeight = space;
    return spacer;
}

function createActionToolButton(actionName) {
    // look for an action
    var cmd = actionManager.command("QtCreator.SaveAll");
    if (cmd === null) {
        jepAPI.debug("Cannot find such action: %1".arg(actionName));
        return null;
    }

    // get action
    var action = cmd.action();

    // create button
    var bttn = jepAPI.createQObject("QToolButton", null);

    // tune button
    bttn.toolButtonStyle = 2;
    bttn.autoRaise = true;
    bttn.styleSheet = "QToolButton {color: white; }";

    // disable button's minimum width
    bttn.sizePolicy = jepAPI.sizePolicy(13, 0, 1);

    // assign action
    bttn.setDefaultAction(action);

    return bttn;
}

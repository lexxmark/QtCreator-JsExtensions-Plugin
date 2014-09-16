
function createModeSpacer(space) {
    var spacer = jepAPI.createQObject("QWidget", null);
    spacer.minimumHeight = space;
    return spacer;
}

function createActionToolButton(actionName) {
    // look for an action
    var cmd = actionManager.command(actionName);
    if (cmd === null) {
        jepAPI.debug("Cannot find such action: " + actionName);
        return null;
    }

    // get action
    var action = cmd.action();

    // create button
    var bttn = jepAPI.createQObject("QToolButton", null);

    // tune button
    bttn.toolButtonStyle = 2;
    bttn.autoRaise = true;
    bttn.styleSheet = "QToolButton {color: black; }";

    // disable button's minimum width
    bttn.sizePolicy = jepAPI.sizePolicy(13, 0, 1);

    // assign action
    bttn.setDefaultAction(action);

    return bttn;
}

// loads JSON object from <url>
// <onLoaded> is callback function(loadedObj, errorStr)
function loadJSON(url, onLoaded) {

    function onDocLoaded(doc, onLoaded) {
        var responseText = doc.responseText;
        if (responseText.length === 0) {
            onLoaded(null, "Response is empty");
            return;
        }

        try {
            // try to parse
            var loadedObj = JSON.parse(responseText);
            onLoaded(loadedObj, "");
        }
        catch (err) {
            onLoaded(null, err.message);
        }
    }

    var doc = new XMLHttpRequest();
    doc.onreadystatechange = function() {
        if (doc.readyState === XMLHttpRequest.DONE) {
            if (doc.status === 200)
                onDocLoaded(doc, onLoaded);
            else
                onLoaded(null, doc.statusText);
        }
    }

    doc.open("get", url);
    doc.setRequestHeader("Content-Encoding", "UTF-8");
    doc.send();
}


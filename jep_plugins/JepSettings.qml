import QtQuick 2.1

QtObject {
    id: settings

    property string settingsPath: ""

    Component.onCompleted: {
        // NOTE:
        // call load() function in callee's onCompleted handler
        // because the order of onCompleted handlers is undefined

        //load();

        // NOTE:
        // save() function couldn't be called at onDestruction
        // because jepAPI object can be null at this moment

        // register settings saving at plugin shutdown
        jepAPI.aboutToShutdown.connect(settings.save);
    }

    Component.onDestruction: {
        // if jepAPI still exists -> disconnect and save
        if (jepAPI) {
            jepAPI.aboutToShutdown.disconnect(settings.save);
            settings.save();
        }
    }

    function load() {
        if (settingsPath.length === 0) {
            jepAPI.debug("Cannot load settings: <settingsPath> property is undefined.");
            return;
        }

        // load as text
        var settingsContent = jepAPI.loadFile(settingsPath);

        if (settingsContent.length === 0) {
            jepAPI.debug("File '%1' is missing or empty".arg(settingsPath));
            return;
        }

        try {
            // try to parse
            var settingsJsObject = JSON.parse(settingsContent);

            // remove possible <system> properties
            delete settingsJsObject.objectName;
            delete settingsJsObject.settingsPath;

            // copy attributes to settings properties
            jepAPI.JsObject2QObject(settingsJsObject, settings);
        }
        catch (err) {
            jepAPI.debug("Error while parsing '%1' file: '%2'".arg(settingsPath).arg(err.message));
        }
    }

    function save() {
        if (settingsPath.length === 0) {
            jepAPI.debug("Error: settings path is not defined.");
            return;
        }

        // copy settings properties to JsObject
        var settingsJsObject = jepAPI.QObject2JsObject(settings);

        // remove <system> properties
        delete settingsJsObject.objectName;
        delete settingsJsObject.settingsPath;

        jepAPI.saveFile(settingsPath, JSON.stringify(settingsJsObject));
    }

}

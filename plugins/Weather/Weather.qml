import QtQuick 2.1
import QtQuick.XmlListModel 2.0

Rectangle {
    id: main

    width: 70
    height: label.height
    color: "gray"

    // see city id at http://weather.yandex.ru/static/cities.xml
    property int cityId: 25563 // Anadyr
    // request weather interval
    property int reqInterval: 30 // minutes

    XmlListModel {
        id: model
        query: "/forecast/fact"

        XmlRole { name: "temp"; query: "temperature/string()" }
        XmlRole { name: "type"; query: "weather_type_short/string()" }
    }

    XmlListModel {
        id: modelCity
        query: "/forecast"

        XmlRole { name: "city"; query: "@city/string()" }
    }

    Text {
        id: label
        color: "white"
        // two empty lines
        text: "\n"
	}

    Timer {
        id: requestTempTimer
        interval: 1000*60*main.reqInterval
        repeat: true
        triggeredOnStart: true

        onTriggered: main.requestTemp();
    }

    Timer {
        id: waitTempTimer
        interval: 1000

        onTriggered: main.tryGetTemp();
    }

    function requestTemp() {
        var doc = new XMLHttpRequest();

        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE)
            {
                // remove invalid attribute
                var xmlData = doc.responseText.replace("xmlns=\"http://weather.yandex.ru/forecast\" ", "");
                model.xml = xmlData;
                modelCity.xml = xmlData;
                waitTempTimer.start();
            }
        }

        doc.open("get", "http://export.yandex.ru/weather-ng/forecasts/" + main.cityId.toString() + ".xml");
        doc.setRequestHeader("Content-Encoding", "UTF-8");
        doc.send();
    }

    function tryGetTemp() {
        if (model.status == XmlListModel.Ready)
        {
            if (model.count != 1)
            {
                console.log("ambigous data has received.");
            }
            else
            {
                console.assert(modelCity.status == XmlListModel.Ready);

                var text = "%1\u2103 %2\n%3";
                text = text.arg(model.get(0).temp).arg(modelCity.get(0).city).arg(model.get(0).type);
                label.text = text;
            }
        }
        else if (model.status == XmlListModel.Error)
        {
            console.log(model.errorString());
        }
        else
        {
            waitTempTimer.start();
        }
    }

    Component.onCompleted: requestTempTimer.start();
}

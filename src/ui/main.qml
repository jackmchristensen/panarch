import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 1280
    height: 720
    title: "Hello Qt Quick"

    Rectangle {
        anchors.fill: parent
        color: "#202020"

        Column {
            anchors.centerIn: parent

            Text {
                text: "Hello Qt!"
                font.pixelSize: 40
                color: "white"
            }

            Button {
                text: "Click Me!"
                onClicked: console.log("Button Pressed!")
            }
        }
    }
}

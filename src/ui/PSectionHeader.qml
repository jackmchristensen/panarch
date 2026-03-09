import QtQuick
import "."

Item {
  property string text: ""
  width: parent.width
  height: labelText.implicitHeight + 10

  // Divider line
  Rectangle {
    width: parent.width
    height: Theme.borderWidth
    anchors.verticalCenter: parent.verticalCenter
    color: Theme.borderSubtle
  }

  Rectangle {
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    width: labelText.implicitWidth + 10
    height: labelText.implicitHeight + 4
    color: Theme.panel

    Text {
      id: labelText
      anchors.centerIn: parent
      text: parent.parent.text.toUpperCase()
      color: Theme.textDisabled
      font.pixelSize: 9
      font.weight: Font.Bold
      font.letterSpacing: 1.2
    }
  }
}

import QtQuick
import "."


Column {
  id: root

  property string label: ""
  property list<string> listItems: []

  spacing: 4
  width: parent.width

  Text {
    text: root.label
    color: Theme.textSecondary
    font.pixelSize: Theme.bodySmall
    font.weight: Font.DemiBold
  }

  Repeater {
    model: root.listItems
    delegate: Rectangle {
      width: parent.width
      height: pathText.implicitHeight + 8
      color: Theme.card
      radius: Theme.radiusSmall
      border.color: Theme.border
      border.width:Theme.borderWidth

      Text {
        id: pathText
        anchors.fill: parent
        anchors.topMargin: 4
        anchors.leftMargin: 10
        text: modelData
        color: Theme.textSecondary
        font.pixelSize: Theme.bodySmall
        wrapMode: Text.WrapAnywhere
      }
    }
  }
}

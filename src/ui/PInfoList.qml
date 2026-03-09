import QtQuick
import "."


Column {
  id: root

  property string label: ""
  property list<string> listItems: []

  spacing: 3
  width: parent.width
  visible: listItems.length > 0

  Text {
    text: root.label
    color: Theme.textSecondary
    font.pixelSize: Theme.bodySmall
    font.weight: Font.Medium
    bottomPadding: 1
  }

  Repeater {
    model: root.listItems
    delegate: Rectangle {
      width: parent.width
      height: itemText.implicitHeight + 10
      color: Theme.inputBg
      radius: Theme.radiusSmall
      border.color: Theme.border
      border.width:Theme.borderWidth

      Rectangle {
        width: 2
        height: parent.height - 4
        anchors.left: parent.left
        anchors.leftMargin: Theme.borderWidth
        anchors.verticalCenter : parent.verticalCenter
        color: Theme.primary
        opacity: 0.6
        radius: 1
        topLeftRadius: Theme.radiusSmall
        bottomLeftRadius: Theme.radiusSmall
      }

      Text {
        id: itemText
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        text: modelData
        color: Theme.textSecondary
        font.pixelSize: Theme.bodySmall
        font.family: "monospace"
        wrapMode: Text.WrapAnywhere
      }
    }
  }
}

import QtQuick
import QtQuick.Controls
import "."

Button {
  id: root

  property color colorBtn: Theme.card
  property int borderWidthBtn: Theme.borderWidth
  property bool usePointer: true

  hoverEnabled: true

  contentItem: Text {
    text: root.text
    color: root.enabled ? Theme.text : Theme.textDisabled
    font.pixelSize: 13
    font.weight: Font.DemiBold
    elide: Text.ElideRight
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
  }

  background: Rectangle {
    radius: Theme.radius
    border.color: Theme.border
    border.width: root.borderWidthBtn
    color: !root.enabled ? Theme.card
      : root.down ? Theme.pressed
      : root.hovered ? Theme.hover
      : root.colorBtn

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on scale { NumberAnimation { duration: 120 } }
  }

  onPressed: background.scale = 0.98
  onReleased: background.scale = 1.0

  HoverHandler {
    cursorShape: root.usePointer ? Qt.PointingHandCursor : Qt.ArrowCursor
  }

  padding: 10
}

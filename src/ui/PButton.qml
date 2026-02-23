import QtQuick
import QtQuick.Controls
import "."

Button {
  id: root

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
    border.width: Theme.borderWidth
    color: !root.enabled ? Theme.card
      : root.down ? Theme.pressed
      : root.hovered ? Theme.hover
      : Theme.card

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on scale { NumberAnimation { duration: 120 } }
  }

  onPressed: background.scale = 0.98
  onReleased: background.scale = 1.0

  HoverHandler {
    cursorShape: Qt.PointingHandCursor
  }

  padding: 10
}

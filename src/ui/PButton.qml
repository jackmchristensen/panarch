import QtQuick
import QtQuick.Controls

Button {
  id: root
  property color bg: "#61AFEF"
  property color bgHover: "#45A1ED"
  property color bgPressed: "#208FE9"
  property color border: "#DAECFB"
  property color fg: "#E6E8EF"
  property int radius: 8

  hoverEnabled: true

  contentItem: Text {
    text: root.text
    color: root.enabled ? root.fg : "#7A8196"
    font.pixelSize: 13
    font.weight: Font.DemiBold
    elide: Text.ElideRight
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
  }

  background: Rectangle {
    radius: root.radius
    border.color: root.border
    border.width: 1
    color: !root.enabled ? "#141827"
      : root.down ? root.bgPressed
      : root.hovered ? root.bgHover
      : root.bg

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on scale { NumberAnimation { duration: 120 } }
  }

  onPressed: background.scale = 0.98
  onReleased: background.scale = 1.0

  padding: 10
}

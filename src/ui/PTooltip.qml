import QtQuick

Item {
  id: root

  property int tooltipDelay: 1000
  property string tooltip: ""
  property Item target: parent

  HoverHandler {
    id: hover
    parent: root.target
    onHoveredChanged: {
      if (hovered) {
        tooltipTimer.start()
      } else {
        tooltipTimer.stop()
        tooltipBox.visible = false
      }
    }
  }

  Timer {
    id: tooltipTimer
    interval: root.tooltipDelay
    repeat: false
    onTriggered: tooltipBox.visible = true
  }

  Rectangle {
    id: tooltipBox
    visible: false
    z: 999
    parent: root.target
    width: tooltipLabel.implicitWidth + 12
    height: tooltipLabel.implicitHeight + 8
    radius: Theme.radiusSmall
    color: Theme.panel
    border.color: Theme.border
    border.width: Theme.borderWidth

    anchors.bottom: parent.top
    anchors.left: parent.left
    anchors.bottomMargin: 4

    Text {
      id: tooltipLabel
      anchors.centerIn: parent
      text: root.tooltip
      color: Theme.text
      font.pixelSize: Theme.bodySmall
    }
  }
}

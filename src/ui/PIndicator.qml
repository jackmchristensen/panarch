import QtQuick

// Item {
//   id: root
//
//   property color dotColor: "white"
//   property bool visibile: true
//   property string tooltip: ""
//   property int tooltipDisplay: 1000
//
//   implicitWidth: indicator.implicitWidth
//   implicitHeight: indicator.implicitHeight
 
Rectangle {
  id: indicator

  property color dotColor: "#FFFFFF"
  property bool isVisible: true
  property string tooltip: ""
  property int tooltipDelay: 1000

  width: 7
  height: 7
  radius: width / 2
  color: dotColor
  visible: isVisible

  HoverHandler {
    id: dotPayloadsHover;
    onHoveredChanged: {
      if (hovered) {
        tooltipPayloadsDelay.start()
      } else {
        tooltipPayloadsDelay.stop()
        tooltipPayloadsVisible.visible = false
      }
    }
  }

  Timer {
    id: tooltipPayloadsDelay
    interval: indicator.tooltipDelay
    repeat: false
    onTriggered: tooltipPayloadsVisible.visible = true
  }

  Rectangle {
    id: tooltipPayloadsVisible
    visible: false
    z: 999
    width: tooltipPayloadsLabel.implicitWidth + 12
    height: tooltipPayloadsLabel.implicitHeight + 8
    radius: Theme.radiusSmall
    color: Theme.panel
    border.color: Theme.border
    border.width: Theme.borderWidth

    anchors.bottom: parent.top
    anchors.left: parent.left
    anchors.bottomMargin: 4

    Text {
      id: tooltipPayloadsLabel
      anchors.centerIn: parent
      text: indicator.tooltip
      color: Theme.text
      font.pixelSize: Theme.bodySmall
    }
  }
}


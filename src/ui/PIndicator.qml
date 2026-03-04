import QtQuick
import "."

Rectangle {
  id: root

  property color dotColor: "#FFFFFF"
  property bool isVisible: true
  property string tooltip: ""
  property int tooltipDelay: 1000
  property bool showTooltip: true

  width: 7
  height: 7
  radius: width / 2
  color: dotColor
  visible: isVisible

  PTooltip {
    visible: root.showTooltip
    tooltip: root.tooltip
    tooltipDelay: root.tooltipDelay
  }
}

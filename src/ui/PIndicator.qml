import QtQuick
import "."

Rectangle {
  id: root

  property color dotColor: "#FFFFFF"
  property bool isVisible: true
  property bool useTooltip: true
  property string tooltip: ""
  property int tooltipDelay: 1000

  width: 7
  height: 7
  radius: width / 2
  color: dotColor
  visible: isVisible

  PTooltip {
    useTooltip: root.useTooltip
    tooltip: root.tooltip
    tooltipDelay: root.tooltipDelay
  }
}

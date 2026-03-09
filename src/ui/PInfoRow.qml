import QtQuick
import "."

Row {
  property string label: ""
  property string value: ""
  property int labelWidth: 88

  width: parent.width
  spacing: 8
  height: 20

  Text {
    text: label
    color: Theme.textSecondary
    font.pixelSize: Theme.bodySmall
    font.weight: Font.Medium
    width: labelWidth
    height: parent.height
    verticalAlignment: Text.AlignVCenter
  }

  Text {
    text: value.length ? value : "—"
    color: Theme.text
    font.pixelSize: Theme.bodySmall
    wrapMode: Text.NoWrap
    elide: Text.ElideRight
    width: parent.width - labelWidth - 8
    height: parent.height
    verticalAlignment: Text.AlignVCenter
  }
}

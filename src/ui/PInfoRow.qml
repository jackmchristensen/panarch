import QtQuick
import "."

Row {
  property string label: ""
  property string value: ""
  property int labelWidth: 84

  width: parent.width
  spacing: 8

  Text {
    text: label
    color: Theme.textSecondary
    font.pixelSize: Theme.bodySmall
    font.weight: Font.DemiBold
    width: labelWidth
  }

  Text {
    text: value.length ? value : "—"
    color: Theme.text
    font.pixelSize: Theme.bodySmall
    wrapMode: Text.NoWrap
    elide: Text.ElideRight
    width: parent.width - labelWidth - 8
  }
}

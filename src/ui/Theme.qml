pragma Singleton
import QtQuick

QtObject {
  id: Theme
  // spacing scale
  readonly property int s1: 6
  readonly property int s2: 10
  readonly property int s3: 14
  readonly property int s4: 18

  // typography
  readonly property int h1: 20
  readonly property int body: 13

  // colors (dark)
  readonly property color bg: "#00FF00"
  readonly property color panel: "#151822"
  readonly property color card: "#1B2030"
  readonly property color cardHover: "#232A3D"
  readonly property color text: "#E6E8EF"
  readonly property color subtext: "#A9AFC0"
  readonly property color border: "#2B3247"
}

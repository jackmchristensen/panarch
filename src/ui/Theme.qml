pragma Singleton
import QtQuick

QtObject {
  id: root

  property bool darkMode: true

  // Spacing scale
  readonly property int s1: 6
  readonly property int s2: 10
  readonly property int s3: 14
  readonly property int s4: 18

  // Radius / Borders
  readonly property int radius: 4
  readonly property int radiusSmall: radius - 2
  readonly property int radiusLarge: radius + 2
  readonly property int borderWidth: 1
  readonly property int borderWidthThick: 2

  // Typography
  readonly property int h1: 20
  readonly property int h2: 16
  readonly property int body: 13
  readonly property int bodySmall: 11

  // Colors - Background
  readonly property color bg: darkMode ? "#282C34" : "#FAFAFA"
  readonly property color panel: darkMode ? "#21252B" : "#F0F0F0"
  readonly property color card: darkMode ? "#2C313C" : "#FFFFFF"
  readonly property color inputBg: darkMode ? "#1E2227" : "#E8E8E8"
  readonly property color bgDisabled: darkMode ? "#3A3F4B" : "#DDDDDD"

  // Colors - Text
  readonly property color text: darkMode ? "#ABB2BF" : "#383A42"
  readonly property color textSecondary: darkMode ? "#828997" : "#696C77"
  readonly property color textDisabled: darkMode ? "#5C6370" : "#9DA0A6"
  readonly property color textInverse: darkMode ? "#FFFFFF" : "#FFFFFF"
  readonly property color textAccent: darkMode ? "#61AFEF" : "#4078F2"

  // Colors - Borders
  readonly property color border: darkMode ? "#4B5263" : "#4B5263"
  readonly property color borderSubtle: darkMode ? "#3A3F4B" : "#4B5263"
  readonly property color borderInput: darkMode ? "#181A1F" : "#C5C5C5"

  // Colors - Accent
  readonly property color primary: darkMode ? "#61AFEF" : "#4078F2"
  readonly property color secondary: darkMode ? "#C678DD" : "#A626A4"

  // Colors - Status
  readonly property color success: darkMode ? "#98C379" : "#98C379"
  readonly property color warning: darkMode ? "#E5C07B" : "#C18401"
  readonly property color error: darkMode ? "#E06C75" : "#E45649"

  // Colors - Interaction
  readonly property color hover: darkMode ? "#3E4451" : "#E2E6F0"
  readonly property color pressed: darkMode ? "#4B5263" : "#CCD4E0"
  readonly property color focus: darkMode ? "#61AFEF" : "#4078F2"
  readonly property color selection: darkMode ? "#3E4451" : "#D7E2FF"

  function toggleTheme() {
    darkMode = !darkMode
  }
}

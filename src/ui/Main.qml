import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "."

ApplicationWindow {
  visible: true
  width: 1200
  height: 720
  title: "Panarch USD Asset Manager"

  id: root
  property int radius: 6
  property int borderWidth: 1
  property color borderColor: "#979797"

  property int textSize: 16
  property color textColor: "#E6E8EF"

  property color bg: "#282C34"

  background: Rectangle {
    anchors.fill: parent
    color: root.bg
  }

  header: ToolBar {
    background: Rectangle { color: background.color }

    RowLayout {
      // anchors.fill: parent
      spacing: 10

      // Label {
      //   text: "Panarch USD Asset Manager"
      //   font.pixelSize: root.textSize
      //   color: root.textColor
      //
      //   Layout.leftMargin: 12
      //   Layout.topMargin: 6
      //   Layout.bottomMargin: 6
      // }

      PButton {
        text: "File"
        font.pixelSize: root.textSize

        Text { color: root.textColor }
        Layout.leftMargin: 12
        Layout.topMargin: 6
        Layout.bottomMargin: 6
      }

      PButton {
        text: "Edit"
        font.pixelSize: root.textSize

        Text { color: root.textColor }
        // Layout.leftMargin: 12
        // Layout.topMargin: 6
        // Layout.bottomMargin: 6
      }

      PButton {
        text: "Add Library..."
        onClicked: folderDialog.open()
      }

      FolderDialog {
        id: folderDialog
        title: "Select Asset Library Root"
        onAccepted: {
          backend.addLibraryRoot(selectedFolder.toString().replace("file://", ""))
        }
      }
    }
  }

  // menuBar: MenuBar {
  //   background: Rectangle { color: root.bg }
  //
  //   Menu {
  //     title: qsTr("&File")
  //     Action { text: qsTr("&Import...") }
  //     MenuSeparator { }
  //     Action { text: qsTr("&Close...") }
  //   }
  // }

  RowLayout {
    anchors.fill: parent
    anchors.margins: 12
    spacing: 12

    ColumnLayout {
      spacing: 12

      Rectangle {
        width: 64
        height: 64
        radius: root.radius
        color: root.bg
        border.color: "#979797"
        border.width: root.borderWidth
      }

      Rectangle {
        width: 64
        height: 64
        radius: root.radius
        color: root.bg
        border.color: "#979797"
        border.width: root.borderWidth
      }
 
      Rectangle {
        width: 64
        height: 64
        radius: root.radius
        color: root.bg
        border.color: "#979797"
        border.width: root.borderWidth
      }
    }

    SplitView {
      anchors.fill: parent
      orientation: Qt.Horizontal

      handle: Rectangle {
        implicitWidth: 6
        color: root.bg
        Rectangle {
          anchors.centerIn: parent
          width: 2
          height: parent.height
          color: "#535353"
          radius: 1
        }
      }

      Frame {
        SplitView.preferredWidth: 700
        SplitView.minimumWidth: 320
        Layout.fillWidth: true
        Layout.fillHeight: true

        background: Rectangle {
          radius: root.radius
          color: root.bg
          border.color: root.borderColor
          border.width: root.borderWidth
        }

        Label {
          text: "Asset Grid" 
          color: "#E6E8EF"
        }

        GridView {
          id: assetGrid
          anchors.fill: parent
          anchors.margins: 12
          cellWidth: 140
          cellHeight: 180
          model: backend.assets

          delegate: Item {
            width: assetGrid.cellWidth
            height: assetGrid.cellHeight

            Column {
              anchors.centerIn: parent
              spacing: 8

              // Thumbnail
              Rectangle {
                width: 120
                height: 120
                color: "#1B2030"
                border.color: "#2B3247"
                radius: 6

                Image {
                  anchors.fill: parent
                  anchors.margins: 4
                  source: model.thumbnail ? "file://" + model.thumbnail : ""
                  fillMode: Image.PreserveAspectFit
                }

                // Fallback icon when no thumbnail
                Text {
                  anchors.centerIn: parent
                  text: "📦"
                  font.pixelSize: 48
                  visible: !model.thumbnail
                }
              }

              // Asset name
              Text {
                width: 120
                text: model.name
                color: "#E6E8EF"
                font.pixelSize: 12
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignHCenter
              }
            }

            MouseArea {
              anchors.fill: parent
              onClicked: backend.selectIndex(index)
            }
          }
        }
      }
 
      Frame {
        SplitView.preferredWidth: 360
        SplitView.minimumWidth: 260
        Layout.maximumWidth: 600
        Layout.fillHeight: true

        background: Rectangle {
          radius: root.radius
          color: root.bg
          border.color: root.borderColor
          border.width: root.borderWidth
        }
 
        Label {
          text: "Inspector" 
          color: "#E6E8EF"
        }

        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 12
          spacing: 12

          Label {
            text: "Selected Asset"
            font.pixelSize: 16
            font.bold: true
            color: root.textColor
          }

          Column {
            spacing: 8
            visible: backend.selectedPath !== ""

            Text {
              text: "Name: " + backend.selectedName
              color: root.textColor
            }

            Text {
              text: "Path: " + backend.selectedPath
              color: "#A9AFC0"
              font.pixelSize: 11
              wrapMode: Text.WrapAnywhere
            }
          }

          Item { Layout.fillHeight: true }
        }
      }
    }
  }
}

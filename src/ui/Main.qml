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

  background: Rectangle {
    anchors.fill: parent
    color: Theme.bg
  }

  header: ToolBar {
    background: Rectangle { color: Theme.bg }

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
        font.pixelSize: Theme.h2

        Layout.leftMargin: 12
        Layout.topMargin: 6
        Layout.bottomMargin: 6
      }

      PButton {
        text: "Edit"
        font.pixelSize: Theme.h2
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
        radius: Theme.radius
        color: Theme.bg
        border.color: Theme.border
        border.width: Theme.borderWidth
      }

      Rectangle {
        width: 64
        height: 64
        radius: Theme.radius
        color: Theme.bg
        border.color: Theme.border
        border.width: Theme.borderWidth
      }
 
      Rectangle {
        width: 64
        height: 64
        radius: Theme.radius
        color: Theme.bg
        border.color: Theme.border
        border.width: Theme.borderWidth
      }
    }

    SplitView {
      anchors.fill: parent
      orientation: Qt.Horizontal

      handle: Rectangle {
        implicitWidth: 6
        color: Theme.bg
        Rectangle {
          anchors.centerIn: parent
          width: 2
          height: parent.height
          color: Theme.borderSubtle
          radius: Theme.radiusSmall
        }
      }

      Frame {
        SplitView.preferredWidth: 700
        SplitView.minimumWidth: 320
        Layout.fillWidth: true
        Layout.fillHeight: true

        background: Rectangle {
          radius: Theme.radius
          color: Theme.bg
          border.color: Theme.border
          border.width: Theme.borderWidth
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
                color: Theme.card
                border.color: Theme.border
                radius: Theme.radius

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
                color: Theme.text
                font.pixelSize: Theme.body
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
          radius: Theme.radius
          color: Theme.bg
          border.color: Theme.border
          border.width: Theme.borderWidth
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
            font.pixelSize: Theme.h2
            font.bold: true
            color: Theme.text
          }

          Column {
            spacing: 8
            visible: backend.selectedPath !== ""

            Text {
              text: "Name: " + backend.selectedName
              color: Theme.text
            }

            Text {
              text: "Path: " + backend.selectedPath
              color: Theme.textSecondary
              font.pixelSize: Theme.bodySmall 
              wrapMode: Text.WrapAnywhere
            }
          }

          Item { Layout.fillHeight: true }
        }
      }
    }
  }
}

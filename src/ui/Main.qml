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
      spacing: 10

      PButton {
        Layout.leftMargin: 6
        Layout.topMargin: Theme.s3
        text: "Add Library..."
        onClicked: folderDialog.open()
      }

      PButton {
        Layout.topMargin: Theme.s3
        text: "File"
        onClicked: dropdown.open()
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

  SplitView {
    anchors.fill: parent
    anchors.margins: 12
    orientation: Qt.Horizontal

    handle: Rectangle {
      implicitWidth: 6
      color: Theme.bg
      Rectangle {
        anchors.centerIn: parent
        width: 2
        height: parent.height - (Theme.radius*2)
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
        color: Theme.panel
        border.color: Theme.border
        border.width: Theme.borderWidth
      }

      // TextField {
      //   anchors.horizontalCenter: parent.horizontalCenter
      //   placeholderText: qsTr("Filter")
      //
      //   background: Rectangle {
      //     width: parent.width
      //     radius: Theme.radius
      //     color: "#FFFFFF"
      //   }
      // }

      GridView {
        id: assetGrid
        anchors.fill: parent
        anchors.margins: Theme.s3
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
              id: thumb
              width: 120
              height: 120
              color: model.path === backend.selectedPath ? Theme.selection
              : mouseFlag.containsMouse ? Theme.hover
              : Theme.card
              border.color: model.path === backend.selectedPath ? Theme.primary : Theme.border
              border.width: model.path === backend.selectedPath ? Theme.borderWidthThick : Theme.borderWidth
              radius: Theme.radius

              Behavior on color { ColorAnimation { duration: 120 } }
              Behavior on scale { NumberAnimation { duration: 120 } }

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
            id: mouseFlag
            anchors.fill: parent
            onClicked: backend.selectIndex(index)
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onPressed: thumb.scale = 0.99
            onReleased: thumb.scale = 1.00
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
        color: Theme.panel
        border.color: Theme.border
        border.width: Theme.borderWidth
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
          width: parent.width

          // Thumbnail preview
          Rectangle {
            width: parent.width
            height: width
            color: Theme.card
            border.color: Theme.border
            radius: Theme.radius

            Image {
              anchors.fill: parent
              anchors.margins: 8
              source: backend.selectedThumbnail ? "file://" + backend.selectedThumbnail : ""
              fillMode: Image.PreserveAspectFit
            }

            Text {
              anchors.centerIn: parent
              text: "📦"
              font.pixelSize: 64
              visible: !backend.selectedThumbnail
            }
          }

          Text {
            text: backend.selectedName
            color: Theme.text
            font.pixelSize: Theme.h2
            font.bold: true
            width: parent.width
            wrapMode: Text.WrapAnywhere
          }

          Text {
            text: backend.selectedType.toUpperCase()
            color: Theme.textAccent
            font.pixelSize: Theme.bodySmall
          }

          Text {
            text: backend.selectedPath
            color: Theme.textSecondary
            font.pixelSize: Theme.bodySmall 
            wrapMode: Text.WrapAnywhere
            width: parent.width
          }
        }

        // Empty state
        // Would like to replace this frame with a drawer in the future
        // which would allow for the panel to not be rendered when empty
        Text {
          text: "No asset selected"
          color: Theme.textDisabled
          font.pixelSize: Theme.body
          visible: backend.selectedPath === ""
        }

        Item { Layout.fillHeight: true }
      }
    }
  }
}

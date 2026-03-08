import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
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

  MouseArea {
    anchors.fill: parent
    onClicked: filter.focus = false
    z: -1
  }

  FolderDialog {
    id: folderDialog
    title: "Select a Library Folder"
    onAccepted: backend.addLibraryRoot(selectedFolder.toString().replace("file://", ""))
  }

  Connections {
    target: backend
    function onOpenLibraryDialogRequested() {
      folderDialog.open()
    }
  }

  header: ToolBar {
    implicitHeight: toolBarLayout.implicitHeight + Theme.s2

    background: Rectangle {
      color: Theme.bg

      Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.borderWidth
        color: Theme.borderSubtle
      }
    }

    RowLayout {
      id: toolBarLayout
      anchors.fill: parent
      spacing: 0

      PMenuButton {
        Layout.leftMargin: 5
        Layout.alignment: Qt.AlignVCenter

        label: "File"
        items: [
          { text: "Add Library...", shortcut: "Ctrl+O", action: () => folderDialog.open() },
          { separator: true },
          { text: "Quit", shortcut: "Ctrl+Q", action: () => backend.quitApp() }
        ]
      }

      PMenuButton {
        Layout.leftMargin: 0
        Layout.alignment: Qt.AlignVCenter

        label: "Edit"
        items: [
          { text: "Settings", shortcut: "Ctrl+,", action: () => console.log("Settings") },
        ]
      }
 
      PMenuButton {
        Layout.leftMargin: 0
        Layout.alignment: Qt.AlignVCenter

        label: "Help"
        items: [
          { text: "About", shortcut: "", action: () => console.log("About") },
        ]
      }

      MouseArea {
        Layout.fillWidth: true
        Layout.fillHeight: true
        onClicked: filter.focus = false
        z: -1
      }

      PButton {
        id: toggle
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        Layout.rightMargin: 5
        Layout.minimumWidth: toggle.implicitHeight
        paddingBtn: 5
        textSize: Theme.bodySmall
        radius: Theme.radiusSmall
        text: "󰔎"
        onClicked: Theme.toggleTheme()
      }
 
    }
  }

  SplitView {
    anchors.fill: parent
    anchors.leftMargin: 12
    anchors.rightMargin: 12
    anchors.bottomMargin: 12
    anchors.topMargin: 5
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
      SplitView.preferredWidth: 746
      SplitView.minimumWidth: 320
      Layout.fillWidth: true
      Layout.fillHeight: true

      background: Rectangle {
        radius: Theme.radius
        color: Theme.panel
        border.color: Theme.border
        border.width: Theme.borderWidth
      }

      MouseArea {
        anchors.fill: parent
        onClicked: filter.focus = false
        z: -1
      }

      Item {
        anchors.fill: parent

        TextField {
          id: filter

          focus: false
          activeFocusOnPress: true
          Keys.onEscapePressed: focus = false

          anchors.horizontalCenter: parent.horizontalCenter
          anchors.top: parent.top
          anchors.topMargin: 15
          width: 280
          placeholderText: qsTr("Filter...")

          leftInset: -8
          rightInset: -8

          color: Theme.text
          placeholderTextColor: Theme.textSecondary

          background: Rectangle {
            implicitWidth: 200
            implicitHeight: 32
            radius: implicitHeight / 2
            color: Theme.inputBg
            border.color: filter.activeFocus ? Theme.primary : Theme.border
            border.width: Theme.borderWidth

            // layer.enabled: true
          }

        }

        // ──── AssetGrid ────────────────────────────────────────────
        GridView {
          id: assetGrid
          interactive: false
          anchors.top: filter.bottom
          anchors.topMargin: 10
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom

          anchors.margins: Theme.s3
          cellWidth: 140
          cellHeight: 180
          model: backend.assets

          displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 250}
          }

          delegate: Item {
            id: delegateRoot
            width: assetGrid.cellWidth
            height: assetGrid.cellHeight
 
            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.mimeData: ({ "text/uri-list": "file://" + model.path })

            Column {
              anchors.centerIn: parent
              spacing: 8

              // ──── Thumbnail ────────────────────────────────────────────
              Rectangle {
                id: thumb
                width: 120
                height: 120
                color: model.path === backend.selectedPath ? Theme.selection
                : mouseFlag.hovered ? Theme.hover
                : Theme.card
                border.color: model.path === backend.selectedPath ? Theme.primary : Theme.border
                border.width: model.path === backend.selectedPath ? Theme.borderWidthThick : Theme.borderWidth
                radius: Theme.radius

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on scale { NumberAnimation { duration: 120 } }

                Image {
                  id: imageThumb
                  anchors.fill: parent
                  anchors.margins: 4
                  source: model.thumbnail ? "file://" + model.thumbnail : ""
                  sourceSize.width: 120
                  sourceSize.height: 120
                  fillMode: Image.PreserveAspectFit

                  layer.enabled: true
                  layer.effect: OpacityMask {
                    id: opacityMask
                    maskSource: Rectangle {
                      id: maskedRect
                      width: imageThumb.width
                      height: imageThumb.height
                      radius: Theme.radiusSmall
                    }
                  }
                }

                // Fallback icon when no thumbnail
                Text {
                  anchors.centerIn: parent
                  text: "📦️"
                  font.pixelSize: 48
                  visible: !model.thumbnail
                }

                // Kind chip
                Rectangle {
                  visible: model.kind !== ""
                  anchors.bottom: parent.bottom
                  anchors.right: parent.right
                  anchors.margins: 5
                  width: kindLabel.implicitWidth + 8
                  height: 16
                  radius: Theme.radiusSmall
                  color: "#88000000"

                  Text {
                    id: kindLabel
                    anchors.centerIn: parent
                    text: model.kind
                    color: Theme.textKindChip
                    font.pixelSize: Theme.bodySmall
                    font.bold:true
                  }
                }

                Row {
                  anchors.bottom: parent.bottom
                  anchors.left: parent.left
                  anchors.bottomMargin: 10
                  anchors.leftMargin: 8
                  spacing: 3

                  // Variant indicator
                  PIndicator {
                    dotColor: Theme.purple
                    isVisible: model.hasVariants
                    tooltip: "Has variants"
                  }

                  // Payload indicator
                  PIndicator {
                    dotColor: Theme.yellow
                    isVisible: model.hasPayloads
                    tooltip: "Has payloads"
                  }

                  // Reference indicator
                  PIndicator {
                    dotColor: Theme.cyan
                    isVisible: model.hasReferences
                    tooltip: "Has references"
                  }
                }
 
                layer.enabled: mouseFlag.hovered || model.path === backend.selectedPath
                layer.effect: DropShadow {
                  transparentBorder: true
                  horizontalOffset: 0
                  verticalOffset: model.path === backend.selectedPath ? 0 : 3
                  radius: 10
                  samples: 17
                  color: model.path === backend.selectedPath ? Theme.primary : "#50000000"

                  // Behavior on color { ColorAnimation { duration: 120 } }
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

            DragHandler {
              id: dragHandler
              target: null
              dragThreshold: 12
            }

            TapHandler {
              onTapped: {
                backend.selectIndex(index)
                filter.focus = false
              }
              onPressedChanged: {
                thumb.scale = pressed ? 0.97 : 1.0
                if (!pressed) Drag.drop()
              }
            }

            HoverHandler {
              id: mouseFlag
              onHoveredChanged: {
                // if (hovered) {
                //   delegateRoot.grabToImage(function(result) {
                //     delegateRoot.Drag.imageSource = result.url
                //   }, Qt.size(70, 90))
                // }
              }
              // cursorShape: Qt.PointingHandCursor
            }

            // MouseArea {
            //   id: mouseFlag
            //   anchors.fill: parent
            //   onClicked: backend.selectIndex(index)
            //   hoverEnabled: true
            //   cursorShape: Qt.PointingHandCursor
            //
            //   onPressed: thumb.scale = 0.99
            //   onReleased: thumb.scale = 1.00
            // }
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

        ScrollView{
          id: infoScroll
          Layout.fillWidth: true
          Layout.fillHeight: true
          clip: true

          Column {
            spacing: Theme.s2
            width: infoScroll.availableWidth

            // Header
            Label {
              text: "Selected Asset"
              font.pixelSize: Theme.h2
              font.bold: true
              color: Theme.text
            }

            // Empty state
            Column {
              visible: backend.selectedPath === ""
              width: parent.width
              spacing: Theme.s1

              Text {
                text: "📂"
                font.pixelSize: 64
                anchors.horizontalCenter: parent.horizontalCenter
              }

              Text {
                text: "No asset selected"
                color: Theme.textDisabled
                font.pixelSize: Theme.body
              }

              Text {
                text: "Select an asset in the grid to see details here."
                color: Theme.textSecondary
                font.pixelSize: Theme.bodySmall
                wrapMode: Text.WrapAnywhere
              }
            }

            // Details state
            Column {
              spacing: Theme.s2
              visible: backend.selectedPath !== ""
              width: parent.width

              // Thumbnail preview
              Rectangle {
                width: parent.width
                height: Math.min(width, 280)
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
                  text: "📦️"
                  font.pixelSize: 64
                  visible: !backend.selectedThumbnail
                }
              }

              // Name + ext chip
              Row {
                width: parent.width
                spacing: 10

                Text {
                  text: backend.selectedName
                  color: Theme.text
                  font.pixelSize: Theme.h2
                  font.bold: true
                  width: parent.width - extChip.width - 10
                  wrapMode: Text.WrapAnywhere
                  elide: Text.ElideRight
                }

                Rectangle {
                  id: extChip
                  radius: Theme.radiusSmall
                  height: 24
                  color: Theme.panel
                  border.color: Theme.border
                  border.width: Theme.borderWidth
                  width: Math.max(52, extLabel.implicitWidth + 16)

                  Text {
                    id: extLabel
                    anchors.centerIn: parent
                    text: backend.selectedExt.toUpperCase()
                    color: Theme.textAccent
                    font.pixelSize: Theme.bodySmall
                    font.bold: true
                  }
                }
              }

              // Info rows
              Column {
                width: parent.width
                spacing: Theme.s1

                PInfoRow { label: "Modified";     value: backend.selectedMTime }
                PInfoRow { label: "Defualt Prim"; value: backend.selectedDefaultPrim }
                PInfoRow { label: "Kind";         value: backend.selectedKind }
                PInfoRow { label: "Size";         value: backend.selectedSize }

                // Path
                Text {
                  text: "Path"
                  color: Theme.textSecondary
                  font.pixelSize: Theme.bodySmall
                  font.weight: Font.DemiBold
                }

                Rectangle {
                  width: parent.width
                  // width: Math.min(parent.width, pathText.implicitWidth + 20)
                  implicitHeight: pathText.implicitHeight + 20
                  radius: Theme.radiusSmall
                  color: Theme.panel
                  border.color: Theme.border
                  border.width: Theme.borderWidth

                  Text {
                    id: pathText
                    anchors.fill: parent
                    anchors.margins: 10
                    text: backend.selectedPath
                    color: Theme.textSecondary
                    font.pixelSize: Theme.bodySmall
                    wrapMode: Text.WrapAnywhere
                  }
                }
              }

              // Actions
              Row {
                width: parent.width
                spacing: 8

                PButton {
                  text: "Copy"
                  onClicked: backend.copySelectedPath()
                }

                PButton {
                  text: "Reveal"
                  onClicked: backend.revealSelected()
                }

                Item {
                  implicitWidth: splitRow.implicitWidth
                  implicitHeight: splitRow.implicitHeight

                  Row {
                    id: splitRow
                    spacing: 0
 
                    PButton {
                      id: openButton
                      text: "Open"
                      borderWidthBtn: 0
                      radiusTR: 0
                      radiusBR: 0
                      radiusBL: menuBtn.popupVisible ? 0 : Theme.radius
                      onClicked: backend.openSelectedUsdview()
                    }

                    Rectangle {
                      width: Theme.borderWidth
                      height: parent.height
                      color: Theme.border
                    }

                    PMenuButton {
                      id: menuBtn
                      label: "▾"
                      labelSize: 13
                      paddingBtn: 10
                      radius: Theme.radius
                      radiusTL: 0
                      radiusBL: 0
                      popupRadiusTL: 0
                      bg: Theme.card
                      xPos: -openButton.implicitWidth - Theme.borderWidth - 0.75
                      items: [
                        { text: "Open in usdview", action: () => backend.openSelectedUsdview() },
                        // { text: "Open in Houdini", action: () => console.debug("Houdini") },
                        // { text: "Open in Maya", action: () => console.debug("Maya") },
                        { text: "Open in Blender", action: () => backend.openSelectedBlender() },
                      ]
                    }
                  }

                  Rectangle {
                    anchors.fill: parent
                    topLeftRadius: Theme.radius
                    topRightRadius: Theme.radius
                    bottomLeftRadius: menuBtn.popupVisible ? 0 : Theme.radius
                    bottomRightRadius: Theme.radius
                    border.color: Theme.border
                    border.width: Theme.borderWidth
                    color: "transparent"
                    z: 1
                  }
                }
              }
            }
          }
        }
      }

      MouseArea {
        anchors.fill: parent
        onClicked: filter.focus = false
        z: -1
      }
    }
  }
}

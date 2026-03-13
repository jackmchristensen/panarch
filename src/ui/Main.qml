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

  Connections {
    target: backend
    function onFocusFilter() {
      filter.forceActiveFocus()
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
      spacing: 1

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

        Row {
          id: filterRow
          anchors.top: parent.top
          anchors.topMargin: 15
          anchors.horizontalCenter: parent.horizontalCenter
          spacing: 15

          TextField {
            id: filter
            anchors.verticalCenter: parent.verticalCenter

            focus: false
            activeFocusOnPress: true
            Keys.onEscapePressed: focus = false
            Keys.onReturnPressed: {
              if (text !== "") backend.selectFirst()
              focus = false
            }
            onTextChanged: backend.filteredAssets.nameFilter = text

            onFocusChanged: {
              if (focus && filter.focusReason !== Qt.MouseFocusReason)
                selectAll()
            }

            // anchors.horizontalCenter: parent.horizontalCenter
            // anchors.top: parent.top
            // anchors.topMargin: 15
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

          PMenuButton {
            anchors.verticalCenter: parent.verticalCenter
            paddingBtn: 8
            border: true
            radius: Theme.radiusSmall
            radiusBL: popupVisible ? 0 : Theme.radiusSmall
            popupRadiusTL: 0

            label: "☰"
            items: [
              { text: "A–Z", action: () => backend.filteredAssets.sortMode = 0 },
              { text: "Z–A", action: () => backend.filteredAssets.sortMode = 1 },
              { text: "New", action: () => backend.filteredAssets.sortMode = 2 },
              { text: "Old", action: () => backend.filteredAssets.sortMode = 3 }
            ]
          }
        }

        // ──── AssetGrid ────────────────────────────────────────────
        GridView {
          id: assetGrid
          interactive: false
          anchors.top: filterRow.bottom
          anchors.topMargin: 10
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          clip: true

          WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
              assetGrid.contentY = Math.max(
                0,
                Math.min(
                  assetGrid.contentY - event.angleDelta.y * 0.15,
                  assetGrid.contentHeight - assetGrid.height
                )
              )
            }
          }

          anchors.margins: Theme.s3
          cellWidth: 140
          cellHeight: 180
          model: backend.filteredAssets

          displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 250; easing.type: Easing.InQuad }
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
                  sourceSize.width: 256
                  sourceSize.height: 256
                  fillMode: Image.PreserveAspectFit

                  layer.enabled: true
                  layer.effect: OpacityMask {
                    maskSource: Rectangle {
                      width: imageThumb.width
                      height: imageThumb.height
                      radius: Theme.radiusSmall
                    }
                  }
                }

                // Fallback icon when no thumbnail
                // Text {
                //   anchors.centerIn: parent
                //   text: "📦️"
                //   font.pixelSize: 48
                //   visible: !model.thumbnail
                // }

                BusyIndicator {
                  id: thumbnailLoading
                  anchors.centerIn: parent
                  running: !model.thumbnail

                  layer.enabled: !model.thumbnail
                  layer.effect: ColorOverlay {
                    color: Theme.primary
                  }
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
                    isVisible: model.hasExternalDeps
                    tooltip: "Has ext. dependencies"
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
                thumb.scale = hovered ? 1.015 : 1.0
                if (hovered) {
                  delegateRoot.grabToImage(function(result) {
                    delegateRoot.Drag.imageSource = result.url
                  },)
                }
              }
            }
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

              // ── Thumbnail ────────────────────────────────────────────
              Rectangle {
                width: parent.width
                height: Math.min(width, 240)
                color: Theme.card
                border.color: Theme.border
                border.width: Theme.borderWidth
                radius: Theme.radius
                clip: true

                Image {
                  id: thumbLarge
                  anchors.fill: parent
                  anchors.margins: Theme.borderWidth
                  source: backend.selectedThumbnail ? "file://" + backend.selectedThumbnail : ""
                  fillMode: Image.PreserveAspectFit
                  visible: backend.selectedThumbnail !== ""

                  layer.enabled: true
                  layer.effect: OpacityMask {
                    id: opacityMask
                    maskSource: Rectangle {
                      id: maskedRect
                      width: thumbLarge.width
                      height: thumbLarge.height
                      radius: Theme.radius
                    }
                  }
                }

                Text {
                  anchors.centerIn: parent
                  text: "📦️"
                  font.pixelSize: 64
                  visible: !backend.selectedThumbnail
                }

              }

              // ── Name + ext chip ──────────────────────────────────────
              Row {
                width: parent.width
                spacing: 8
                // leftPadding: 2

                Text {
                  text: backend.selectedName
                  color: Theme.text
                  font.pixelSize: Theme.h2
                  font.bold: true
                  width: parent.width - extChip.width - 8
                  wrapMode: Text.WrapAnywhere
                  elide: Text.ElideRight
                  anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                  id: extChip
                  radius: Theme.radiusSmall
                  height: 22
                  color: Theme.inputBg
                  border.color: Theme.border
                  border.width: Theme.borderWidth
                  width: Math.max(48, extLabel.implicitWidth + 16)
                  anchors.verticalCenter: parent.verticalCenter

                  Text {
                    id: extLabel
                    anchors.centerIn: parent
                    text: backend.selectedExt.toUpperCase()
                    color: Theme.textAccent
                    font.pixelSize: Theme.bodySmall
                    font.bold: true
                    font.letterSpacing: 0.8
                  }
                }
              }

              // ── Asset metadata ───────────────────────────────────────
              Column {
                id: infoRows
                width: parent.width
                spacing: 2

                property bool showLoading: false

                PSectionHeader { text: "Asset" }

                PInfoRow { label: "Kind";               value: backend.selectedKind }
                PInfoRow { label: "Default Prim Path";  value: backend.selectedDefaultPrim }
                PInfoRow { label: "Modified";           value: backend.selectedMTime }
                PInfoRow { label: "Size";               value: backend.selectedSize }

                // ── Path ───────────────────────────────────────────────
                Column {
                  width: parent.width
                  spacing: 4

                  Text {
                    text: "Path"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.bodySmall
                    font.weight: Font.Medium
                    topPadding: 2
                  }

                  Rectangle {
                    width: parent.width
                    implicitHeight: pathText.implicitHeight + 16
                    radius: Theme.radiusSmall
                    color: Theme.inputBg
                    border.color: Theme.border
                    border.width: Theme.borderWidth

                    Text {
                      id: pathText
                      anchors.fill: parent
                      anchors.margins: 8
                      text: backend.selectedPath
                      color: Theme.textSecondary
                      font.pixelSize: Theme.bodySmall
                      font.family: "monospace"
                      wrapMode: Text.WrapAnywhere
                    }
                  }
                }

                PSectionHeader { text: "Stage" }

                PInfoRow { label: "Up Axis";          value: infoRows.showLoading ? "Loading..." : backend.upAxis }
                PInfoRow { label: "Meters/Unit";  value: infoRows.showLoading ? "Loading..." : backend.metersPerUnit }
                PInfoRow { label: "FPS";  value: infoRows.showLoading ? "Loading..." : backend.framesPerSecond }
                PInfoRow { label: "TCPS";  value: infoRows.showLoading ? "Loading..." : backend.timeCodesPerSecond }
                PInfoRow { label: "Prim Count";  value: infoRows.showLoading ? "Loading..." : backend.primCount }

                PSectionHeader {
                  text: "Composition"
                  visible: backend.sublayers.length > 0
                    || backend.payloads.length > 0
                    || backend.references.length > 0
                }

                Column {
                  width: parent.width
                  spacing: Theme.s1
                  visible: !infoRows.showLoading

                  PInfoList { label: "Sublayers"; listItems: backend.sublayers }
                  PInfoList { label: "Payloads"; listItems: backend.payloads }
                  PInfoList { label: "References"; listItems: backend.references }
                }

                PSectionHeader {
                  text: "Variants"
                  visible: backend.variantSets.length > 0 && !infoRows.showLoading
                }

                Repeater {
                  model: infoRows.showLoading ? [] : backend.variantSets
                  delegate: Column {
                    width: parent.width
                    spacing: 3
                    bottomPadding: 4

                    property var variantSet: modelData

                    Text {
                      text: modelData["name"]
                      color: Theme.textSecondary
                      font.pixelSize: Theme.bodySmall
                      font.weight: Font.Medium
                      leftPadding: 2
                      bottomPadding: 2
                    }

                    Flow {
                      width: parent.width
                      spacing: 4

                      Repeater {
                        model: modelData["variants"]
                        delegate: Rectangle {
                          property bool isSelected: modelData === variantSet["selected"]
                          height: chipLabel.implicitHeight + 8
                          width: chipLabel.implicitWidth + 16
                          radius: Theme.radiusSmall
                          color: isSelected ? Qt.rgba(Theme.primary.r, Theme.primary.g, Theme.primary.b, 0.15) : Theme.inputBg
                          border.color: isSelected ? Theme.primary : Theme.border
                          border.width: Theme.borderWidth

                          Text {
                            id: chipLabel
                            anchors.centerIn: parent
                            text: modelData
                            color: isSelected ? Theme.primary : Theme.textSecondary
                            font.pixelSize: Theme.bodySmall
                          }
                        }
                      }
                    }
                  }
                }

                // ── Loading state timer logic ──────────────────────────
                Timer {
                  id: loadingDelay
                  interval: 200
                  onTriggered: infoRows.showLoading = true
                }

                Connections {
                  target: backend
                  function onSelectedChanged() {
                    infoRows.showLoading = false
                    loadingDelay.restart()
                  }
                  function onLoadingDetailsChanged() {
                    if (!backend.loadingDetails) {
                      loadingDelay.stop()
                      infoRows.showLoading = false
                    }
                  }
                }
              }
            }
          }
        }

        Column {
          visible: backend.selectedPath !== ""
          Layout.fillWidth: true
          spacing: 10

          Rectangle {
            width: parent.width
            height: Theme.borderWidth
            color: Theme.borderSubtle
          }

          // ── Actions ──────────────────────────────────────────────
          Row {
            width: parent.width
            spacing: 8

            Repeater {
              model: backend.actions.filter(a => a.group === "left")
              delegate: PButton {
                text: modelData.label
                onClicked: backend.triggerAction(modelData.id)
              }
            }

            Item {
              visible: backend.actions.some(a => a.group === "split_primary")
              implicitWidth: splitRow.implicitWidth
              implicitHeight: splitRow.implicitHeight

              property var primaryAction: backend.actions.find(a => a.group === "split_primary")
              property var secondaryAction: backend.actions.find(a => a.group === "split_secondary")

              Row {
                id: splitRow
                spacing: 0

                PButton {
                  id: openButton
                  text: parent.parent.primaryAction?.label ?? ""
                  borderWidthBtn: 0
                  radiusTR: 0
                  radiusBR: 0
                  radiusTL: menuBtn.popupVisible ? 0 : Theme.radius
                  onClicked: backend.triggerAction(parent.parent.primaryAction?.id ?? "")
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
                  popupRadiusBL: 0
                  bg: Theme.card
                  xPos: -openButton.implicitWidth - Theme.borderWidth - 0.75
                  items: backend.actions
                    .filter(a => a.group === "split_secondary")
                    .map(a => ({ text: a.label, action: () => backend.triggerAction(a.id) }))
                }
              }

              Rectangle {
                anchors.fill: parent
                topLeftRadius: menuBtn.popupVisible ? 0 : Theme.radius
                topRightRadius: Theme.radius
                bottomLeftRadius: Theme.radius
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

      MouseArea {
        anchors.fill: parent
        onClicked: filter.focus = false
        z: -1
      }
    }
  }
}

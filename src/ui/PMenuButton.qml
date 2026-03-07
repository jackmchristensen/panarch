import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Item {
  id: root

  property string label: "Menu"
  property int labelSize: Theme.bodySmall
  property int paddingBtn: 5
  property var items: []
  property color accentColor: Theme.primary
  property color bg: Theme.bg
  property bool border: false
  property bool popupVisible: popup.visible

  property int radius: Theme.radiusSmall
  property int radiusTL: radius
  property int radiusTR: radius
  property int radiusBL: radius
  property int radiusBR: radius

  property int popupRadiusTL: radius
  property int popupRadiusTR: radius
  property int popupRadiusBL: radius
  property int popupRadiusBR: radius
  
  property int btnWidth: menuBtn.implicitWidth
  property int btnHeight: menuBtn.implicitHeight

  property int xPos: 0
 
  implicitWidth: btnWidth
  implicitHeight: btnHeight

  PButton {
    id: menuBtn
    anchors.centerIn: parent
    text: root.label
    usePointer: false
    colorBtn: root.bg
    paddingBtn: root.paddingBtn
    textSize: root.labelSize
    borderWidthBtn: root.border ? Theme.borderWidth : 0

    radiusTL: root.radiusTL
    radiusTR: root.radiusTR
    radiusBL: root.radiusBL
    radiusBR: root.radiusBR

    onClicked: popup.visible ? popup.close() : popup.open()
  }

  Popup {
    id: popup
    parent: menuBtn
    y: root.height
    x: root.xPos
    width: 200
    padding: 4
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    background: Rectangle {
      color: Theme.card
      border.color: Theme.border
      border.width: Theme.borderWidth

      topLeftRadius: root.popupRadiusTL
      topRightRadius: root.popupRadiusTR
      bottomLeftRadius: root.popupRadiusBL
      bottomRightRadius: root.popupRadiusBR
    }

    contentItem: Column {
      spacing: 0
      width: parent.width

      Repeater {
        model: root.items

        delegate: Loader {
          width: popup.width - 8
          sourceComponent: modelData.separator ? separatorComp : itemComp

          onLoaded: {
            if (!modelData.separator) item.itemData = modelData
          }
        }
      }
    }
  }

  Component {
    id: itemComp

    Rectangle {
      property var itemData: ({})

      width: parent ? parent.width : 0
      height: 28
      color: itemMouse.containsMouse ? Theme.hover : "transparent"
      radius: Theme.radiusSmall

      RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.s2
        anchors.rightMargin: Theme.s2

        Text {
          text: itemData.text ?? ""
          color: Theme.text
          font.pixelSize: Theme.body
          Layout.fillWidth: true
        }

        Text {
          text: itemData.shortcut ?? ""
          color: Theme.textSecondary
          font.pixelSize: Theme.bodySmall
        }
      }

      MouseArea {
        id: itemMouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
          popup.close()
          if (itemData.action) itemData.action()
        }
      }
    }
  }

  Component {
    id: separatorComp

    Rectangle {
      width: parent ? parent.width : 0
      height: 9
      color: "transparent"

      Rectangle {
        width: parent.width - 10
        height: 1
        anchors.centerIn: parent
        color: Theme.border
      }
    }
  }
}

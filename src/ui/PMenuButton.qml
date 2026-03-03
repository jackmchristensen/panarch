import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Item {
  id: root

  property string label: "Menu"
  property var items: []
  property color accentColor: Theme.primary

  implicitWidth: menuBtn.implicitWidth
  implicitHeight: menuBtn.implicitHeight

  PButton {
    id: menuBtn
    anchors.centerIn: parent
    text: root.label
    colorBtn: Theme.bg
    borderWidthBtn: 0

    onClicked: popup.visible ? popup.close() : popup.open()
  }

  Popup {
    id: popup
    parent: menuBtn
    y: root.height
    x: 0
    width: 200
    padding: 4
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    background: Rectangle {
      color: Theme.bg
      border.color: Theme.border
      border.width: Theme.borderWidth
      radius: Theme.radius
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

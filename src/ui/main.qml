import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
  visible: true
  width: 1280
  height: 720
  title: "Panarch USD Asset Manager"

  header: ToolBar {
    RowLayout {
      anchors.fill: parent
      ToolButton {
        text: "Open Library..."
        onClicked: folderDialog.open()
      }
      Label { text: backend.selectedName.length ? ("Selected: " + backend.selectedName) : "Selected: (none)" }
    }
  }

  FolderDialog {
    id: folderDialog
    title: "Choose Asset Library Folder"
    onAccepted: {
      backend.loadLibrary(folderDialog.folder.toString().replace("file://", ""))
    }
  }

  SplitView {
    anchors.fill: parent

    ListView {
      id: list
      model: backend.assets
      clip: true

      delegate: ItemDelegate {
        width: list.width
        text: name + " (" + type + ")"
        onClicked: backend.selectedIndex(index)
      }
    }

    Frame {
      SplitView.preferredWidth: 420
      ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label { text: "Inspector"; font.pixelSize: 18 }

        TextArea {
          Layout.fillWidth: true
          Layout.fillHeight: true
          readOnly: true
          text:
            "Name: " + backend.selectedName + "\n" +
            "Path: " + backend.selectedPath + "\n\n" +
            "Next: USD summary + validation results here."
        }
      }
    }
  }
}

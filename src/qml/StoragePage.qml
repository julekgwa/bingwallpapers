import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import BingApp
import QtQuick.Dialogs

PagePane {
    id: page

    property int pathState: BingIO.dirState(BingIO.wallpaperDir)
    property int lastDays: 30

    function applyPath(path) {
        pathState = BingIO.dirState(path)
        if (pathState === BingIO.DirValid)
            BingIO.wallpaperDir = path
    }

    Component.onCompleted: if (BingIO.deleteDays > 0) lastDays = BingIO.deleteDays

    Connections {
        target: BingIO
        function onSettingsChanged() {
            if (BingIO.deleteDays > 0)
                page.lastDays = BingIO.deleteDays
        }
    }

    SectionCard {
        title: qsTr("Download location")

        FolderField {
            id: folder
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 8
            pathState: page.pathState
            onEdited: (path) => page.pathState = BingIO.dirState(path)
            onSubmitted: (path) => page.applyPath(path)
            onBrowse: folderDialog.open()

            Binding {
                target: folder
                property: "text"
                value: BingIO.wallpaperDir
                when: !folder.editing
            }
        }
    }

    SectionCard {
        title: qsTr("Cleanup")

        ToggleRow {
            text: qsTr("Delete old wallpapers")
            helper: qsTr("Free up space automatically")
            checked: BingIO.deleteDays > 0
            onToggled: (checked) => BingIO.deleteDays = checked ? page.lastDays : 0
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 8
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Delete after")
                color: BingIO.deleteDays > 0 ? Theme.fg2 : Theme.disabledFg
                font.pixelSize: 12
            }
            Stepper {
                name: qsTr("days")
                unit: qsTr("days")
                from: 1
                to: 365
                enabled: BingIO.deleteDays > 0
                value: BingIO.deleteDays > 0 ? BingIO.deleteDays : page.lastDays
                onValueModified: (value) => BingIO.deleteDays = value
            }
        }
    }

    Item { Layout.fillHeight: true }

    FolderDialog {
        id: folderDialog
        title: qsTr("Choose a download folder")
        currentFolder: "file://" + BingIO.wallpaperDir
        onAccepted: {
            const path = BingIO.urlToLocalPath(selectedFolder)
            folder.text = path
            page.applyPath(path)
        }
    }
}

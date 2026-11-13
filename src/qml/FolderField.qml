import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

ColumnLayout {
    id: root

    // Matches BingIO::DirState: 0 valid, 1 missing, 2 permission denied.
    property int pathState: 0
    property alias text: input.text
    readonly property bool editing: input.activeFocus

    signal edited(string path)
    signal submitted(string path)
    signal browse()

    spacing: 6

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 40
        radius: Theme.radiusSm
        color: Theme.surface2
        border.width: 1
        border.color: root.pathState === 0 ? Qt.alpha(Theme.success, 0.5)
                      : root.pathState === 1 ? Qt.alpha(Theme.danger, 0.55)
                      : Qt.alpha(Theme.warn, 0.55)

        RowLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: 40
                Layout.fillHeight: true

                Icon {
                    anchors.centerIn: parent
                    name: "folder"
                    size: 16
                    color: Theme.muted
                }
            }

            TextField {
                id: input
                Layout.fillWidth: true
                Layout.fillHeight: true
                leftPadding: 0
                rightPadding: 8
                verticalAlignment: TextInput.AlignVCenter
                color: Theme.fg
                selectByMouse: true
                font.pixelSize: 12
                background: null
                Accessible.name: qsTr("Download folder")
                onTextEdited: root.edited(text)
                onEditingFinished: root.submitted(text)
            }

            Rectangle {
                Layout.fillHeight: true
                implicitWidth: 1
                color: Theme.borderMed
            }

            AbstractButton {
                id: browseButton
                Layout.fillHeight: true
                leftPadding: 12
                rightPadding: 12
                text: qsTr("Browse…")
                onClicked: root.browse()

                background: Rectangle {
                    color: browseButton.hovered ? Theme.border : "transparent"
                    Rectangle {
                        visible: browseButton.visualFocus
                        anchors.fill: parent
                        anchors.margins: 1
                        color: "transparent"
                        border.width: 2
                        border.color: Theme.accent
                    }
                }
                contentItem: Label {
                    text: browseButton.text
                    color: Theme.fg2
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 5

        readonly property color messageColor: root.pathState === 0 ? Theme.success
                                              : root.pathState === 1 ? Theme.danger
                                              : Theme.warn

        Icon {
            name: root.pathState === 0 ? "check" : root.pathState === 1 ? "alert" : "lock"
            size: 12
            color: parent.messageColor
        }
        Label {
            Layout.fillWidth: true
            text: root.pathState === 0 ? qsTr("Folder is ready to use")
                  : root.pathState === 1 ? qsTr("This folder doesn't exist")
                  : qsTr("No permission to read or write this folder")
            color: parent.messageColor
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }
}

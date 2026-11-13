import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Item {
    id: root

    property string text
    property string helper
    property bool checked: false
    property bool showDivider: false

    signal toggled(bool checked)

    Layout.fillWidth: true
    implicitHeight: row.implicitHeight + 20

    Rectangle {
        visible: root.showDivider
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.border
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: toggle.toggle()
    }

    RowLayout {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: root.text
                color: root.enabled ? Theme.fg : Theme.disabledFg
                font.pixelSize: 13
                elide: Text.ElideRight
            }
            Label {
                Layout.fillWidth: true
                visible: root.helper !== ""
                text: root.helper
                color: Theme.muted
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }

        AppSwitch {
            id: toggle
            enabled: root.enabled
            Accessible.name: root.text
            onToggled: root.toggled(checked)
        }
    }

    // Keeps the switch following the model even after the user has flipped it.
    Binding {
        target: toggle
        property: "checked"
        value: root.checked
    }
}

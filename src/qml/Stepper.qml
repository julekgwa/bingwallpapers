import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

RowLayout {
    id: root

    property int value: 0
    property int from: 0
    property int to: 100
    property string unit
    property string name

    signal valueModified(int value)

    spacing: 8

    function step(delta) {
        const next = Math.max(from, Math.min(to, value + delta))
        if (next !== value)
            valueModified(next)
    }

    component StepButton: AbstractButton {
        id: button

        property string iconName

        implicitWidth: 32
        implicitHeight: 34
        autoRepeat: true
        autoRepeatDelay: 400
        autoRepeatInterval: 80

        background: Rectangle {
            color: button.hovered && button.enabled ? Theme.border : "transparent"
            Rectangle {
                visible: button.visualFocus
                anchors.fill: parent
                anchors.margins: 1
                color: "transparent"
                border.width: 2
                border.color: Theme.accent
            }
        }
        contentItem: Item {
            Icon {
                anchors.centerIn: parent
                name: button.iconName
                size: 14
                color: button.enabled ? Theme.fg2 : Theme.disabledFg
            }
        }
    }

    Rectangle {
        implicitWidth: 32 + 38 + 32 + 2
        implicitHeight: 36
        radius: Theme.radiusSm
        color: Theme.surface2
        border.width: 1
        border.color: Theme.borderMed
        opacity: root.enabled ? 1 : 0.5
        clip: true

        RowLayout {
            anchors.fill: parent
            anchors.margins: 1
            spacing: 0

            StepButton {
                iconName: "minus"
                enabled: root.enabled && root.value > root.from
                Accessible.name: qsTr("Decrease %1").arg(root.name)
                onClicked: root.step(-1)
            }
            Label {
                Layout.preferredWidth: 38
                text: root.value
                color: root.enabled ? Theme.fg : Theme.disabledFg
                font.pixelSize: 13
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                Accessible.name: root.name
            }
            StepButton {
                iconName: "plus"
                enabled: root.enabled && root.value < root.to
                Accessible.name: qsTr("Increase %1").arg(root.name)
                onClicked: root.step(1)
            }
        }
    }

    Label {
        visible: root.unit !== ""
        text: root.unit
        color: root.enabled ? Theme.muted : Theme.disabledFg
        font.pixelSize: 11
    }
}

import QtQuick
import QtQuick.Controls.Basic

Switch {
    id: control

    implicitWidth: 40
    implicitHeight: 22
    padding: 0

    indicator: Rectangle {
        width: 40
        height: 22
        radius: 11
        color: control.checked ? Theme.accent : Theme.toggleOff
        opacity: control.enabled ? 1 : 0.5

        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            width: 16
            height: 16
            radius: 8
            y: 3
            x: control.checked ? 21 : 3
            color: "#ffffff"

            Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
        }

        Rectangle {
            visible: control.visualFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: height / 2 + 3
            color: "transparent"
            border.width: 2
            border.color: Theme.accent
        }
    }

    contentItem: Item {}
}

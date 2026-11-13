import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

AbstractButton {
    id: control

    property bool primary: true
    property string iconName
    property bool busy: false
    property string busyText

    implicitHeight: 40
    leftPadding: 14
    rightPadding: 14

    background: Rectangle {
        radius: Theme.radiusSm
        color: control.primary
               ? (control.hovered && !control.busy ? Theme.accentHover : Theme.accent)
               : (control.hovered ? Theme.border : "transparent")
        border.width: control.primary ? 0 : 1
        border.color: Theme.borderMed

        Rectangle {
            visible: control.visualFocus
            anchors.fill: parent
            anchors.margins: -3
            radius: parent.radius + 3
            color: "transparent"
            border.width: 2
            border.color: Theme.accent
        }
    }

    contentItem: Item {
        implicitWidth: control.busy ? Math.max(112, busyLabel.implicitWidth) : row.implicitWidth
        implicitHeight: control.implicitHeight

        RowLayout {
            id: row
            visible: !control.busy
            anchors.centerIn: parent
            spacing: 6

            Icon {
                visible: control.iconName !== ""
                name: control.iconName
                size: 15
                color: control.primary ? Theme.accentText : Theme.fg2
            }
            Label {
                text: control.text
                color: control.primary ? Theme.accentText : Theme.fg
                font.pixelSize: 13
                font.weight: Font.DemiBold
            }
        }

        ColumnLayout {
            visible: control.busy
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 4

            Label {
                id: busyLabel
                text: control.busyText
                color: Theme.accentText
                opacity: 0.85
                font.pixelSize: 11
                font.weight: Font.Medium
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 3
                radius: 2
                color: Qt.rgba(1, 1, 1, 0.28)
                clip: true

                Rectangle {
                    id: fill
                    height: parent.height
                    radius: 2
                    color: Qt.rgba(1, 1, 1, 0.9)

                    SequentialAnimation on width {
                        running: control.busy
                        loops: Animation.Infinite
                        NumberAnimation { to: fill.parent.width * 0.72; duration: 700; easing.type: Easing.InOutQuad }
                        NumberAnimation { to: fill.parent.width * 0.15; duration: 700; easing.type: Easing.InOutQuad }
                    }
                }
            }
        }
    }
}

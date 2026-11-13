import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    property var model: []
    property int currentIndex: -1

    signal activated(int index)

    Layout.fillWidth: true
    implicitHeight: 34
    radius: Theme.radiusSm
    color: Theme.surface2
    border.width: 1
    border.color: Theme.borderMed
    opacity: enabled ? 1 : 0.5
    clip: true

    RowLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0

        Repeater {
            model: root.model

            delegate: AbstractButton {
                id: button

                readonly property bool selected: index === root.currentIndex

                Layout.fillWidth: true
                Layout.fillHeight: true
                text: modelData
                enabled: root.enabled
                onClicked: root.activated(index)

                background: Rectangle {
                    color: button.selected ? Theme.accent : (button.hovered ? Theme.border : "transparent")
                    radius: Theme.radiusXs

                    Rectangle {
                        visible: button.visualFocus
                        anchors.fill: parent
                        color: "transparent"
                        radius: parent.radius
                        border.width: 2
                        border.color: Theme.accent
                    }
                }

                contentItem: Label {
                    text: button.text
                    color: button.selected ? Theme.accentText : Theme.fg2
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}

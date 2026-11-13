import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    property string title
    default property alias content: body.data

    Layout.fillWidth: true
    implicitHeight: column.implicitHeight
    color: Theme.surface
    radius: Theme.radius
    border.width: 1
    border.color: Theme.border
    clip: true

    ColumnLayout {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Label {
            Layout.fillWidth: true
            Layout.leftMargin: 14
            Layout.topMargin: 9
            Layout.bottomMargin: 7
            text: root.title.toUpperCase()
            color: Theme.muted
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 0.8
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: Theme.border
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            Layout.leftMargin: 14
            Layout.rightMargin: 14
            Layout.topMargin: 4
            Layout.bottomMargin: 6
            spacing: 0
        }
    }
}

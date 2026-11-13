import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

ComboBox {
    id: control

    // Model entries look like { name, code, flag }.
    textRole: "name"
    implicitHeight: 40
    leftPadding: 12
    rightPadding: 36

    readonly property string currentFlag: currentIndex >= 0 && model[currentIndex] ? model[currentIndex].flag : ""

    background: Rectangle {
        radius: Theme.radiusSm
        color: Theme.surface2
        border.width: 1
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

    indicator: Icon {
        x: control.width - width - 12
        y: (control.height - height) / 2
        name: "chevron"
        size: 14
        color: Theme.muted
    }

    contentItem: RowLayout {
        spacing: 10

        Image {
            source: control.currentFlag
            sourceSize.height: 40
            Layout.preferredHeight: 15
            Layout.preferredWidth: 20
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        Label {
            Layout.fillWidth: true
            text: control.displayText
            color: Theme.fg
            font.pixelSize: 13
            elide: Text.ElideRight
        }
    }

    delegate: ItemDelegate {
        id: item

        required property var modelData
        required property int index

        width: control.width - 8
        height: 36
        highlighted: control.highlightedIndex === index

        background: Rectangle {
            radius: Theme.radiusXs
            color: item.highlighted ? Theme.accentDim : "transparent"
        }
        contentItem: RowLayout {
            spacing: 10

            Image {
                source: item.modelData.flag
                sourceSize.height: 40
                Layout.preferredHeight: 15
                Layout.preferredWidth: 20
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
            Label {
                Layout.fillWidth: true
                text: item.modelData.name
                color: Theme.fg
                font.pixelSize: 13
                font.weight: item.index === control.currentIndex ? Font.DemiBold : Font.Normal
            }
        }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 280)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle {
            radius: Theme.radiusSm
            color: Theme.surface
            border.width: 1
            border.color: Theme.borderMed
        }
    }
}

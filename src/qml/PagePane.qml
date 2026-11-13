import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A tab page: scrolls internally, never the whole window.
ScrollView {
    id: root

    default property alias content: column.data

    padding: 12
    clip: true
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    // Always visible while the page is longer than the window, so it is
    // obvious that there is more below.
    ScrollBar.vertical: ScrollBar {
        id: bar
        parent: root
        x: root.width - width - 2
        y: root.topPadding
        height: root.availableHeight
        policy: size < 1.0 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        implicitWidth: 6

        contentItem: Rectangle {
            implicitWidth: 4
            radius: 2
            color: Theme.muted
            opacity: bar.pressed ? 0.9 : 0.55
        }
        background: null
    }

    ColumnLayout {
        id: column
        width: root.availableWidth
        spacing: 8
    }
}

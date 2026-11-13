import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Shapes
import Qt.labs.platform as Platform
import BingApp

ApplicationWindow {
    id: root

    property string toastText
    property bool toastOk: true

    width: 420
    height: 660
    minimumWidth: 360
    minimumHeight: 520
    title: qsTr("Bing Wallpapers")
    color: Theme.bg

    readonly property string licenseUrl: "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html"
    readonly property string websiteUrl: "https://bingwallpapers.github.io"

    function showWindow() {
        show()
        raise()
        requestActivate()
    }

    function toggleWindow() {
        if (visible)
            hide()
        else
            showWindow()
    }

    function showInfo() {
        showWindow()
        infoPopup.open()
    }

    Component.onCompleted: {
        // Autostart keeps the window in the tray unless there is no tray.
        if (!startHidden || !tray.available)
            show()
    }

    onClosing: (close) => {
        if (tray.available) {
            close.accepted = false
            hide()
        } else {
            Qt.quit()
        }
    }

    Connections {
        target: BingIO
        function onRefreshFinished(ok, message) {
            root.toastOk = ok
            root.toastText = message
            toastTimer.restart()
        }
    }

    Timer {
        id: toastTimer
        interval: 4000
        onTriggered: root.toastText = ""
    }

    // ------------------------------------------------------------------ tray

    Platform.SystemTrayIcon {
        id: tray
        icon.source: "qrc:/resources/bing.png"
        visible: true
        tooltip: qsTr("Bing Wallpapers – next refresh %1").arg(BingIO.nextRefresh)
        onActivated: (reason) => {
            if (reason === Platform.SystemTrayIcon.Trigger)
                root.toggleWindow()
        }

        menu: Platform.Menu {
            Platform.MenuItem {
                text: qsTr("Show Bing Wallpapers")
                icon.source: "qrc:/resources/bing.png"
                onTriggered: root.showWindow()
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: qsTr("Next refresh: %1").arg(BingIO.nextRefresh)
                icon.source: "qrc:/resources/refresh-hover.png"
                enabled: false
            }
            Platform.MenuItem {
                text: qsTr("Refresh now")
                icon.source: "qrc:/resources/refresh-hover.png"
                onTriggered: BingIO.refresh()
            }
            Platform.MenuItem {
                text: qsTr("About today's wallpaper")
                icon.source: "qrc:/resources/info.png"
                onTriggered: root.showInfo()
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: qsTr("Quit")
                icon.source: "qrc:/resources/quit.png"
                onTriggered: Qt.quit()
            }
        }
    }

    // ---------------------------------------------------------------- layout

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Offline banner
        Rectangle {
            Layout.fillWidth: true
            visible: !BingIO.online
            implicitHeight: bannerRow.implicitHeight + 14
            color: Theme.dangerDim

            RowLayout {
                id: bannerRow
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 8

                Icon { name: "offline"; size: 14; color: Theme.dangerText }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("<b>No internet connection.</b> Wallpapers refresh when you're back online.")
                    textFormat: Text.StyledText
                    color: Theme.dangerText
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }

        // Pinned header: wallpaper preview, refresh and status
        Item {
            id: hero

            readonly property bool empty: BingIO.wallpaperUrl.toString() === ""

            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 10
            Layout.preferredHeight: Math.max(110, Math.min(width * 9 / 16, root.height * 0.22))

            Rectangle {
                anchors.fill: parent
                radius: Theme.radius
                color: Theme.surface2
            }

            Image {
                anchors.fill: parent
                visible: !hero.empty
                source: BingIO.wallpaperUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: false
                sourceSize.width: 900
                smooth: true
            }

            // First run: nothing downloaded yet
            Rectangle {
                anchors.fill: parent
                visible: hero.empty
                radius: Theme.radius
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0; color: "#1c1c2e" }
                    GradientStop { position: 1; color: "#0f3460" }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        implicitWidth: 48
                        implicitHeight: 48
                        radius: 12
                        color: Qt.rgba(1, 1, 1, 0.07)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.12)

                        Icon {
                            anchors.centerIn: parent
                            name: "image"
                            size: 22
                            color: Qt.rgba(1, 1, 1, 0.4)
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("No wallpaper yet")
                        color: Qt.rgba(1, 1, 1, 0.55)
                        font.pixelSize: 12
                    }
                    ActionButton {
                        Layout.alignment: Qt.AlignHCenter
                        implicitHeight: 34
                        text: qsTr("Download today's wallpaper")
                        enabled: !BingIO.busy
                        onClicked: BingIO.refresh()
                    }
                }
            }

            // Title / copyright over a gradient
            Rectangle {
                visible: !hero.empty && BingIO.wallpaperTitle !== ""
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: Math.min(parent.height, overlayRow.implicitHeight + 28)
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0; color: "transparent" }
                    GradientStop { position: 1; color: Qt.rgba(0, 0, 0, 0.75) }
                }

                RowLayout {
                    id: overlayRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                    spacing: 8

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Label {
                            Layout.fillWidth: true
                            text: BingIO.wallpaperTitle
                            color: Qt.rgba(1, 1, 1, 0.95)
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Label {
                            Layout.fillWidth: true
                            visible: BingIO.wallpaperCopyright !== ""
                            text: BingIO.wallpaperCopyright
                            color: Qt.rgba(1, 1, 1, 0.65)
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    AbstractButton {
                        id: infoButton
                        implicitWidth: 28
                        implicitHeight: 28
                        Accessible.name: qsTr("About this wallpaper")
                        onClicked: infoPopup.open()

                        background: Rectangle {
                            radius: 14
                            color: infoButton.hovered ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0, 0, 0, 0.4)
                            border.width: infoButton.visualFocus ? 2 : 1
                            border.color: infoButton.visualFocus ? Theme.accent : Qt.rgba(1, 1, 1, 0.3)
                        }
                        contentItem: Item {
                            Icon {
                                anchors.centerIn: parent
                                name: "info"
                                size: 14
                                color: Qt.rgba(1, 1, 1, 0.9)
                            }
                        }
                    }
                }
            }

            // Rounded corners: cover the corners of the square image with the page colour.
            Shape {
                anchors.fill: parent
                layer.enabled: true
                layer.samples: 4

                ShapePath {
                    id: corners

                    fillColor: Theme.bg
                    strokeColor: "transparent"
                    fillRule: ShapePath.OddEvenFill

                    readonly property real w: hero.width
                    readonly property real h: hero.height
                    readonly property real r: Theme.radius

                    PathSvg {
                        path: "M0 0H" + corners.w + "V" + corners.h + "H0Z "
                              + "M" + corners.r + " 0H" + (corners.w - corners.r)
                              + "A" + corners.r + " " + corners.r + " 0 0 1 " + corners.w + " " + corners.r
                              + "V" + (corners.h - corners.r)
                              + "A" + corners.r + " " + corners.r + " 0 0 1 " + (corners.w - corners.r) + " " + corners.h
                              + "H" + corners.r
                              + "A" + corners.r + " " + corners.r + " 0 0 1 0 " + (corners.h - corners.r)
                              + "V" + corners.r
                              + "A" + corners.r + " " + corners.r + " 0 0 1 " + corners.r + " 0Z"
                    }
                }
            }
        }

        // Refresh row
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 10
            Layout.bottomMargin: 8
            spacing: 8

            ActionButton {
                iconName: "refresh"
                text: qsTr("Refresh now")
                busy: BingIO.busy
                busyText: qsTr("Downloading…")
                enabled: !BingIO.busy
                onClicked: BingIO.refresh()
            }

            Item { Layout.fillWidth: true }

            Icon { name: "clock"; size: 13; color: Theme.muted }
            Label {
                text: qsTr("Next: %1").arg(BingIO.nextRefresh)
                color: Theme.muted
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        // Refresh result
        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.bottomMargin: 8
            visible: root.toastText !== ""
            implicitHeight: toastRow.implicitHeight + 14
            radius: Theme.radiusSm
            color: root.toastOk ? Theme.successDim : Theme.dangerDim
            border.width: 1
            border.color: Qt.alpha(root.toastOk ? Theme.success : Theme.danger, 0.22)

            RowLayout {
                id: toastRow
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Icon {
                    name: root.toastOk ? "check" : "alert"
                    size: 13
                    color: root.toastOk ? Theme.success : Theme.dangerText
                }
                Label {
                    Layout.fillWidth: true
                    text: root.toastText
                    color: root.toastOk ? Theme.success : Theme.dangerText
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    wrapMode: Text.WordWrap
                }
            }
        }

        // Tabs
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            implicitHeight: 44

            background: Rectangle {
                color: Theme.tabBarBg
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: Theme.border
                }
            }

            component AppTab: TabButton {
                id: tab

                property string iconName

                implicitHeight: 44

                background: Item {
                    Rectangle {
                        visible: tab.checked
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 0.6
                        height: 2
                        radius: 1
                        color: Theme.accent
                    }
                    Rectangle {
                        visible: tab.visualFocus
                        anchors.fill: parent
                        anchors.margins: 3
                        radius: 4
                        color: "transparent"
                        border.width: 2
                        border.color: Theme.accent
                    }
                }
                contentItem: Item {
                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 5

                        Icon {
                            name: tab.iconName
                            size: 15
                            color: tab.checked ? Theme.accent : Theme.muted
                        }
                        Label {
                            text: tab.text
                            color: tab.checked ? Theme.accent : Theme.muted
                            font.pixelSize: 12
                            font.weight: Font.Medium
                        }
                    }
                }
            }

            AppTab { text: qsTr("Wallpaper"); iconName: "image" }
            AppTab { text: qsTr("Schedule"); iconName: "calendar" }
            AppTab { text: qsTr("Storage"); iconName: "folder" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            WallpaperPage {}
            SchedulePage {}
            StoragePage {}
        }

        // Footer
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            color: Theme.chrome

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.border
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 7

                Image {
                    source: "qrc:/resources/bing.png"
                    sourceSize.height: 44
                    Layout.preferredWidth: 22
                    Layout.preferredHeight: 22
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("Bing Wallpapers")
                    color: Theme.fg2
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }
                LinkButton {
                    text: qsTr("About")
                    onClicked: aboutPopup.open()
                }
            }
        }
    }

    // --------------------------------------------------------------- dialogs

    component LinkButton: AbstractButton {
        id: link

        padding: 4
        background: Rectangle {
            color: "transparent"
            border.width: link.visualFocus ? 2 : 0
            border.color: Theme.accent
            radius: 3
        }
        contentItem: Label {
            text: link.text
            color: Theme.accent
            font.pixelSize: 12
            font.underline: link.hovered
        }
        HoverHandler { cursorShape: Qt.PointingHandCursor }
    }

    component ModalCard: Popup {
        anchors.centerIn: Overlay.overlay
        width: 300
        padding: 0
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.5) }
        enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 120 } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100 } }
        background: Rectangle {
            radius: Theme.radius
            color: Theme.surface
            border.width: 1
            border.color: Theme.borderMed
        }
    }

    ModalCard {
        id: aboutPopup

        contentItem: ColumnLayout {
            spacing: 0

            RowLayout {
                Layout.margins: 16
                spacing: 10

                Image {
                    source: "qrc:/resources/bing.png"
                    sourceSize.height: 80
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
                ColumnLayout {
                    spacing: 0
                    Label {
                        text: qsTr("Bing Wallpapers")
                        color: Theme.fg
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Version %1").arg(BingIO.version)
                        color: Theme.muted
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Label { Layout.fillWidth: true; text: qsTr("License"); color: Theme.muted; font.pixelSize: 13 }
                    LinkButton {
                        text: qsTr("GPL v2+")
                        font.pixelSize: 13
                        onClicked: Qt.openUrlExternally(root.licenseUrl)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label { Layout.fillWidth: true; text: qsTr("Website"); color: Theme.muted; font.pixelSize: 13 }
                    LinkButton {
                        text: "bingwallpapers.github.io"
                        onClicked: Qt.openUrlExternally(root.websiteUrl)
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("Distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.")
                    color: Theme.muted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
            }

            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 12

                Item { Layout.fillWidth: true }
                ActionButton {
                    primary: false
                    implicitHeight: 34
                    text: qsTr("Close")
                    onClicked: aboutPopup.close()
                }
            }
        }
    }

    ModalCard {
        id: infoPopup

        contentItem: ColumnLayout {
            spacing: 0

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    text: BingIO.wallpaperTitle !== "" ? BingIO.wallpaperTitle : qsTr("Today's wallpaper")
                    color: Theme.fg
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
                Label {
                    Layout.fillWidth: true
                    text: BingIO.wallpaperCopyright !== "" ? BingIO.wallpaperCopyright
                          : BingIO.wallpaperTitle !== "" ? "" : qsTr("No details are available for this wallpaper.")
                    visible: text !== ""
                    color: Theme.muted
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 12

                Item { Layout.fillWidth: true }
                ActionButton {
                    primary: false
                    implicitHeight: 34
                    text: qsTr("Close")
                    onClicked: infoPopup.close()
                }
            }
        }
    }
}

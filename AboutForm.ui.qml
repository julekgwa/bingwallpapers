import QtQuick 2.4
import QtQuick.Layouts 1.0

BackgroundPage {
    width: 400
    height: 500

    Image {
        id: image
        x: 126
        y: 22
        width: 150
        height: 150
        fillMode: Image.PreserveAspectFit
        source: "resources/bing.png"
    }

    Text {
        id: text1
        x: 158
        y: 183
        color: "#ffffff"
        text: qsTr("Version: 1.0")
        font.pixelSize: 16
    }

    Text {
        id: text2
        x: 51
        y: 210
        color: "#ffffff"
        text: qsTr("Set bing wallpaper as desktop background")
        font.pixelSize: 16
    }

    CustomLabel {
        id: license
        x: 38
        y: 266
        color: "#ffffff"
        text: qsTr("<a href='https://www.gnu.org/licenses/old-licenses/gpl-2.0.html'>GNU General Public License, version 2 or later</a>")
        font.pixelSize: 16
        // @disable-check M222
        onLinkActivated: Qt.openUrlExternally(
                             "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html")

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }

    Text {
        id: text3
        x: 178
        y: 251
        color: "#ffffff"
        text: qsTr("License:")
        font.pixelSize: 12
    }

    CustomLabel {
        id: webpage
        x: 170
        y: 310
        color: "#ffffff"
        text: qsTr("<a href='bingwallpapers.lekgoara.com'>Website</a>")
        font.pixelSize: 16
        // @disable-check M222
        onLinkActivated: Qt.openUrlExternally(
                             "http://bingwallpapers.lekgoara.com")

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }

    GridLayout {
        x: 58
        y: 398
        rows: 1
        columns: 2

        Text {
            id: author
            color: "#ffffff"
            text: qsTr("Created by Junius")
            font.pixelSize: 16
        }

        CustomLabel {
            id: email
            text: "<a href='mailto:phutigravel@gmail.com?Subject=Bing%20Wallpapers'>(phutigravel@gmail.com)</a>"
            // @disable-check M222
            onLinkActivated: Qt.openUrlExternally("mailto:phutigravel@gmail.com?Subject=Bing%20Wallpapers")

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }
}

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

BackgroundPage {
    width: 400
    height: 500
    color: "#191919"
    property alias setLockScreen: setLockScreen
    property alias setBackgroundImage: setBackgroundImage
    property alias deleteDaysComboBox: deleteDaysComboBox
    property alias deleteWallpapers: deleteWallpapers
    property alias countryLogo: countryLogo
    property alias countries: countries
    clip: false
    title: "Settings"

    GridLayout {
        x: 8
        y: 390
        width: 384
        height: 48
        rows: 1
        columns: 3
    }

    Frame {
        id: frame
        x: 8
        y: 89
        width: 384
        height: 169

        Text {
            id: text2
            x: -2
            y: 1
            color: "#ffffff"
            text: qsTr("Set background image")
            font.pixelSize: 16
        }

        Text {
            id: text3
            x: -3
            y: 41
            color: "#ffffff"
            text: qsTr("Set lock screen image")
            font.pixelSize: 16
        }

        Switch {
            id: setBackgroundImage
            x: 310
            y: -12
            text: qsTr("")
            checked: true
        }

        Switch {
            id: setLockScreen
            x: 310
            y: 28
            text: qsTr("")
        }

        Text {
            id: text4
            x: 0
            y: 84
            color: "#ffffff"
            text: qsTr("Delete previously downloaded wallpapers")
            font.pixelSize: 16
        }

        Switch {
            id: deleteWallpapers
            x: 310
            y: 71
            text: qsTr("")
        }

        Text {
            id: text5
            x: 0
            y: 123
            color: "#ffffff"
            text: qsTr("Delete wallpapers after")
            font.pixelSize: 16
        }

        ComboBox {
            id: deleteDaysComboBox
            x: 291
            y: 115
            width: 69
            height: 38
            textRole: ""
            enabled: false
            model: 31
            currentIndex: BingIO.get_days_to_delete_pic
        }
    }

    Frame {
        id: frame1
        x: 8
        y: 276
        width: 384
        height: 162

        ComboBox {
            id: countries
            x: 51
            y: 97
            width: 309
            height: 48
            Layout.fillWidth: true
            implicitWidth: 257
            model: ["United States", "United Kingdom", "Deutsch", "Canada", "Australia", "France", "日本", "中国"]
            // @disable-check M222
            currentIndex: BingIO.set_combo_box_region()
        }

        Text {
            id: text1
            x: -7
            y: 112
            color: "#ffffff"
            text: qsTr("Country:")
            font.pixelSize: 14
        }

        Image {
            id: countryLogo
            x: 130
            y: 0
            width: 100
            height: 100
            visible: true
            source: "resources/us.png"
        }


    }
}

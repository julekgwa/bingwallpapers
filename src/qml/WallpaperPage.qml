import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import BingApp

PagePane {
    id: page

    readonly property var regions: [
        { name: qsTr("United States"), code: "en-US", flag: "qrc:/resources/us.png" },
        { name: qsTr("United Kingdom"), code: "en-GB", flag: "qrc:/resources/uk.png" },
        { name: qsTr("Germany"), code: "de-DE", flag: "qrc:/resources/germany.png" },
        { name: qsTr("Canada"), code: "en-CA", flag: "qrc:/resources/canada.png" },
        { name: qsTr("Australia"), code: "en-AU", flag: "qrc:/resources/australia.png" },
        { name: qsTr("France"), code: "fr-FR", flag: "qrc:/resources/france.png" },
        { name: qsTr("Japan"), code: "ja-JP", flag: "qrc:/resources/japan.png" },
        { name: qsTr("China"), code: "zh-CN", flag: "qrc:/resources/china.png" }
    ]

    SectionCard {
        title: qsTr("Apply to")

        ToggleRow {
            text: qsTr("Desktop background")
            helper: qsTr("Sets the new wallpaper as your desktop")
            checked: BingIO.setBackground
            onToggled: (checked) => BingIO.setBackground = checked
        }
        ToggleRow {
            showDivider: true
            text: qsTr("Lock screen")
            helper: qsTr("Also applies to the login / lock screen")
            checked: BingIO.setLockScreen
            onToggled: (checked) => BingIO.setLockScreen = checked
        }
    }

    SectionCard {
        title: qsTr("Picture")

        RegionComboBox {
            id: regionBox
            Layout.fillWidth: true
            Layout.topMargin: 10
            model: page.regions
            Accessible.name: qsTr("Region")
            onActivated: (index) => BingIO.region = page.regions[index].code

            Binding {
                target: regionBox
                property: "currentIndex"
                value: Math.max(0, page.regions.findIndex(r => r.code === BingIO.region))
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.topMargin: 6
            Layout.bottomMargin: 6
            text: qsTr("Region takes effect on the next refresh")
            color: Theme.muted
            font.pixelSize: 11
        }

        ToggleRow {
            showDivider: true
            text: qsTr("Ultra HD (4K)")
            helper: qsTr("Sharper, about 3 MB a day. Refresh now to switch.")
            checked: BingIO.ultraHd
            onToggled: (checked) => BingIO.ultraHd = checked
        }
    }
}

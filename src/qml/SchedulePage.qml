import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import BingApp

PagePane {
    id: page

    readonly property var presets: [5, 10, 15, 30, 60]
    readonly property int customSegment: presets.length
    readonly property bool rotating: BingIO.rotateMinutes > 0
    readonly property int presetIndex: presets.indexOf(BingIO.rotateMinutes)

    // The interval to come back to when rotation is switched on again.
    property int lastMinutes: 30
    property bool customMode: false
    property int customHours: 0
    property int customMinutes: 30

    readonly property int segmentIndex: !rotating ? -1
                                        : (customMode || presetIndex < 0) ? customSegment
                                        : presetIndex
    readonly property bool showCustom: rotating && segmentIndex === customSegment
    readonly property int customTotal: customHours * 60 + customMinutes
    readonly property bool customTooShort: customTotal < BingIO.minRotateMinutes

    function syncFromBackend() {
        const minutes = BingIO.rotateMinutes
        if (minutes <= 0)
            return
        lastMinutes = minutes
        customHours = Math.floor(minutes / 60)
        customMinutes = minutes % 60
    }

    function applyCustom() {
        if (!customTooShort)
            BingIO.rotateMinutes = customTotal
    }

    Component.onCompleted: syncFromBackend()

    Connections {
        target: BingIO
        function onSettingsChanged() { page.syncFromBackend() }
    }

    SectionCard {
        title: qsTr("Rotation")

        ToggleRow {
            text: qsTr("Rotate through saved wallpapers")
            helper: qsTr("Cycle your downloaded collection automatically")
            checked: page.rotating
            onToggled: (checked) => {
                if (!checked)
                    page.customMode = false
                BingIO.rotateMinutes = checked ? Math.max(page.lastMinutes, BingIO.minRotateMinutes) : 0
            }
        }

        Segmented {
            Layout.topMargin: 2
            Layout.bottomMargin: 8
            enabled: page.rotating
            model: [qsTr("5 min"), qsTr("10 min"), qsTr("15 min"), qsTr("30 min"), qsTr("1 h"), qsTr("Custom")]
            currentIndex: page.segmentIndex
            Accessible.name: qsTr("Rotation interval")
            onActivated: (index) => {
                if (index === page.customSegment) {
                    page.customMode = true
                    page.applyCustom()
                } else {
                    page.customMode = false
                    BingIO.rotateMinutes = page.presets[index]
                }
            }
        }

        ColumnLayout {
            visible: page.showCustom
            Layout.fillWidth: true
            Layout.bottomMargin: 8
            spacing: 6

            RowLayout {
                spacing: 8
                Label {
                    Layout.preferredWidth: 56
                    text: qsTr("Hours")
                    color: Theme.fg2
                    font.pixelSize: 12
                }
                Stepper {
                    name: qsTr("hours")
                    from: 0
                    to: 23
                    value: page.customHours
                    onValueModified: (value) => { page.customHours = value; page.applyCustom() }
                }
            }
            RowLayout {
                spacing: 8
                Label {
                    Layout.preferredWidth: 56
                    text: qsTr("Minutes")
                    color: Theme.fg2
                    font.pixelSize: 12
                }
                Stepper {
                    name: qsTr("minutes")
                    from: 0
                    to: 59
                    value: page.customMinutes
                    onValueModified: (value) => { page.customMinutes = value; page.applyCustom() }
                }
            }
            RowLayout {
                visible: page.customTooShort
                spacing: 4

                Icon { name: "alert"; size: 12; color: Theme.dangerText }
                Label {
                    text: qsTr("Minimum %1 minutes").arg(BingIO.minRotateMinutes)
                    color: Theme.dangerText
                    font.pixelSize: 11
                }
            }
        }
    }

    Item { Layout.fillHeight: true }
}

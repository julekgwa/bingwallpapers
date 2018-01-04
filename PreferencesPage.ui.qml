import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4 as Exclusive

BackgroundPage {
    width: 400
    height: 500
    property alias fiveRadionBtn: fiveRadionBtn
    property alias tenRadioBtn: tenRadionBtn
    property alias fifteenRadionBtn: fifteenRadionBtn
    property alias thirtyRadionBtn: thirtyRadionBtn
    property alias sixtyRadionBtn: sixtyRadionBtn
    property alias customRadionBtn: customRadionBtn
    property alias rotateImagesSwitch: rotateImagesSwitch
    property alias rotateGroupBox: rotateGroupBox
    property alias downloadFolderComboBox: downloadFolderComboBox
    property alias customIntervalText: customIntervalText
    property alias rotateButtonGroup: rotateButtonGroup
    title: "Customize BW"

    ButtonGroup {
        id: rotateButtonGroup
    }
    GroupBox {
        id: rotateGroupBox
        x: 26
        y: 118
        width: 349
        height: 200
        enabled: false
        visible: true
        title: qsTr("Rotation Intervals")


        RadioButton {
            id: fiveRadionBtn
            x: 0
            y: 0
            text: qsTr("5 Minutes")
            ButtonGroup.group: rotateButtonGroup
        }

        RadioButton {
            id: tenRadionBtn
            x: 201
            y: 0
            text: qsTr("10 Minutes")
            ButtonGroup.group: rotateButtonGroup
        }

        RadioButton {
            id: fifteenRadionBtn
            x: 0
            y: 54
            text: qsTr("15 Minutes")
            ButtonGroup.group: rotateButtonGroup
        }

        RadioButton {
            id: thirtyRadionBtn
            x: 201
            y: 54
            text: qsTr("30 Minutes")
            ButtonGroup.group: rotateButtonGroup
        }

        RadioButton {
            id: sixtyRadionBtn
            x: 0
            y: 117
            text: qsTr("60 Minutes")
            checked: false
            ButtonGroup.group: rotateButtonGroup
        }

        RadioButton {
            id: customRadionBtn
            x: 201
            y: 117
            text: qsTr("Custom")
            ButtonGroup.group: rotateButtonGroup
        }
    }

    Switch {
        id: rotateImagesSwitch
        x: 26
        y: 64
        text: qsTr("Rotate images")
    }

    Text {
        id: text1
        x: 26
        y: 355
        color: "#ffffff"
        text: qsTr("Download folder:")
        font.pixelSize: 16
    }

    ComboBox {
        id: downloadFolderComboBox
        x: 27
        y: 382
        width: 348
        height: 48
    }

    CustomLabel {
        id: customIntervalText
        x: 214
        y: 314
        width: 161
        height: 21
        //color: "#a59a9a"
        text: qsTr("")
        font.italic: true
        visible: true
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 13
    }
}

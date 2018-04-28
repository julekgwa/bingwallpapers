import QtQuick 2.4
import QtQuick.Controls 2.0

BackgroundPage {
    width: 300
    height: 800
    property alias button1: button1
    property alias comboBox: comboBox
    title: "Smart Home"

    CustomLabel {
        id: customLabel1
        x: 31
        y: 69
        text: "12:23"
        font.pixelSize: 22
    }

    Label {
        id: label1
        x: 34
        y: 112
        text: qsTr("Sunday")
        font.pixelSize: 12
    }

    CustomLabel {
        id: customLabel2
        x: 31
        y: 145
        text: "12 April 2016"
        font.pixelSize: 18
    }

    Label {
        id: label2
        x: 31
        y: 187
        text: qsTr("Temperature")
        font.pixelSize: 12
    }

    CustomLabel {
        id: customLabel3
        x: 31
        y: 219
        text: "18 Degrees"
        font.pixelSize: 18
    }

    Label {
        id: label3
        x: 34
        y: 299
        text: qsTr("Power Consumption")
        font.pixelSize: 12
    }

    ProgressBar {
        id: progressBar1
        x: 31
        y: 337
        value: 0.5
    }

    Switch {
        id: switch1
        x: 31
        y: 369
        text: qsTr("Ventilation")
    }

    Slider {
        id: slider1
        x: 31
        y: 434
        value: 0.5
    }

    Switch {
        id: switch2
        x: 34
        y: 495
        text: qsTr("Alarm Active")
    }

    RadioButton {
        id: radioButton1
        x: 31
        y: 558
        text: qsTr("Active Alert")
    }

    Label {
        id: label4
        x: 31
        y: 639
        text: qsTr("Mode")
        font.pixelSize: 12
    }

    ComboBox {
        id: comboBox
        x: 31
        y: 671
        width: 212
        height: 43
    }

    Button {
        id: button1
        x: 31
        y: 728
        width: 212
        text: qsTr("Configure")
    }
}

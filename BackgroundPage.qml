import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Rectangle {
    Material.theme: Material.Dark
    Material.accent: Material.DeepOrange
    width: 400
    height: 500
    color: "#191919"
    clip: true
    property alias title: label.text

    Label {
        id: label
        x: 8
        y: 18
        text: qsTr("")
        font.pixelSize: 22
    }

    Rectangle {
        id: rectangle1
        x: 312
        y: -106
        width: 200
        height: 200
        color: "#3a3a3a"
        z: 1
        clip: true
        rotation: 45
    }


}

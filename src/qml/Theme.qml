pragma Singleton
import QtQuick

// Design tokens. Dark is the default; the light set follows the system colour scheme.
QtObject {
    readonly property bool dark: Qt.styleHints.colorScheme !== Qt.Light

    readonly property color bg: dark ? "#181818" : "#f0f0f0"
    readonly property color surface: dark ? "#242424" : "#ffffff"
    readonly property color surface2: dark ? "#2c2c2c" : "#f5f5f5"
    readonly property color chrome: dark ? "#141414" : "#e0e0e0"
    readonly property color tabBarBg: dark ? "#1e1e1e" : "#e8e8e8"
    readonly property color fg: dark ? "#e8e8e8" : "#1a1a1a"
    readonly property color fg2: dark ? "#b0b0b0" : "#3a3a3a"
    readonly property color muted: dark ? "#808080" : "#666666"
    readonly property color border: dark ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color borderMed: dark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.14)

    readonly property color accent: "#0F8FA8"
    readonly property color accentHover: "#0d7d94"
    readonly property color accentDim: Qt.rgba(15 / 255, 143 / 255, 168 / 255, dark ? 0.18 : 0.12)
    readonly property color accentText: "#ffffff"

    readonly property color success: "#3d9970"
    readonly property color successDim: Qt.rgba(61 / 255, 153 / 255, 112 / 255, dark ? 0.15 : 0.12)
    readonly property color warn: "#e09b2d"
    readonly property color danger: "#d9534f"
    readonly property color dangerDim: Qt.rgba(217 / 255, 83 / 255, 79 / 255, dark ? 0.15 : 0.10)
    readonly property color dangerText: dark ? "#e07070" : "#c0392b"

    readonly property color toggleOff: dark ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(0, 0, 0, 0.18)
    readonly property color disabledFg: dark ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(0, 0, 0, 0.28)

    readonly property int radius: 12
    readonly property int radiusSm: 8
    readonly property int radiusXs: 6
}

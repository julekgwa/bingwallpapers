import QtQuick
import QtQuick.Shapes

// Stroke icons (24x24 grid) drawn with Shapes so they can be recoloured freely.
Item {
    id: root

    property string name
    property color color: Theme.fg
    property real size: 16

    implicitWidth: size
    implicitHeight: size

    readonly property var paths: ({
        "refresh": "M21 12a9 9 0 1 1-9-9c2.52 0 4.93 1 6.74 2.74L21 8 M21 3v5h-5",
        "clock": "M12 2a10 10 0 1 0 0 20 10 10 0 1 0 0-20 M12 6v6l4 2",
        "info": "M12 2a10 10 0 1 0 0 20 10 10 0 1 0 0-20 M12 16v-4 M12 8h.01",
        "alert": "M12 2a10 10 0 1 0 0 20 10 10 0 1 0 0-20 M12 8v4 M12 16h.01",
        "image": "M5 3h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2Z M11 9a2 2 0 1 0-4 0 2 2 0 0 0 4 0 M21 15l-3.086-3.086a2 2 0 0 0-2.828 0L6 21",
        "calendar": "M8 2v4 M16 2v4 M5 4h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2Z M3 10h18",
        "folder": "M20 20a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-7.9a2 2 0 0 1-1.69-.9L9.6 3.9A2 2 0 0 0 7.93 3H4a2 2 0 0 0-2 2v13a2 2 0 0 0 2 2Z",
        "check": "M20 6L9 17l-5-5",
        "lock": "M5 11h14a2 2 0 0 1 2 2v7a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-7a2 2 0 0 1 2-2Z M7 11V7a5 5 0 0 1 10 0v4",
        "chevron": "M6 9l6 6 6-6",
        "minus": "M5 12h14",
        "plus": "M5 12h14 M12 5v14",
        "x": "M18 6L6 18 M6 6l12 12",
        "offline": "M12 20h.01 M8.5 16.429a5 5 0 0 1 7 0 M5 12.859a10 10 0 0 1 5.17-2.69 M19 12.859a10 10 0 0 0-2.007-1.523 M2 8.82a15 15 0 0 1 4.177-2.643 M22 8.82a15 15 0 0 0-11.288-3.764 M2 2l20 20"
    })

    Shape {
        width: 24
        height: 24
        scale: root.size / 24
        transformOrigin: Item.TopLeft
        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeColor: root.color
            strokeWidth: 2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathSvg { path: root.paths[root.name] || "" }
        }
    }
}

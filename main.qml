import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4 as Tray
import QtQuick.Dialogs 1.2
import QtQuick.Extras 1.2
import QtQuick.Window 2.0
import QSystemTrayIcon 1.0

ApplicationWindow {
    visible: true
    id: application
    width: 400
    height: 500
    title: qsTr("Bing Wallpapers")

    Timer {
        id: timer1
        interval: 300000
        repeat: true
        running: false

        onTriggered: {
            BingIO.run_script()
        }
    }

    // delay script by five seconds, on startup
    Timer {
        id: delayScript
        interval: 10000
        repeat: false
        running: false

        onTriggered: {
            BingIO.run_script()
        }
    }

    Component.onCompleted: {
        delayScript.running = true
    }

    // system tray

    QSystemTrayIcon {
        id: systemTray

        // Initial initialization of the system tray
        Component.onCompleted: {
            icon = iconTray             // Set icon
            toolTip = "Bingwallpapers"
            show();
            if(application.visibility === Window.Hidden) {
                application.show()
            } else {
                application.hide()
            }
        }

        /* By clicking on the tray icon define the left or right mouse button click was.
         * If left, then hide or open the application window.
         * If right, then open the System Tray menu
        * */
        onActivated: {
            if(reason === 1){
                trayMenu.popup()
            } else {
                if(application.visibility === Window.Hidden) {
                    application.show()
                } else {
                    application.hide()
                }
            }
        }
    }

    // Menu system tray
    Tray.Menu {
        id: trayMenu

        Tray.MenuItem {
            text: qsTr("Show BW")
            onTriggered: application.show()
        }

        Tray.MenuItem {
            text: qsTr("Quit")
            onTriggered: {
                systemTray.hide()
                Qt.quit()

            }
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Connections {
            target: BingIO
            onData_changed: {
                BingIO.save_data()
            }
        }

        DefaultPage {
            setLockScreen.onCheckedChanged: {
                BingIO.set_lock_screen(setLockScreen.checked)
            }
            setBackgroundImage.onCheckedChanged: {
                BingIO.set_background_image(setBackgroundImage.checked)
            }
            deleteDaysComboBox.onCurrentIndexChanged: {
                BingIO.set_days_to_delete_pic(deleteDaysComboBox.currentIndex)
            }
            deleteWallpapers.onCheckedChanged: {
                deleteDaysComboBox.enabled = !deleteDaysComboBox.enabled
                if (!deleteWallpapers.checked) {
                    BingIO.set_days_to_delete_pic(false)
                }
            }

            Component.onCompleted: {
                var deleteDays = BingIO.get_days_to_delete_pic
                setBackgroundImage.checked = BingIO.get_background_image
                setLockScreen.checked = BingIO.get_lock_screen
                deleteWallpapers.checked = deleteDays > 0 ? true : false
                if (deleteDays) {
                    deleteDaysComboBox.currentIndex = deleteDays
                }
            }

            countries.onCurrentIndexChanged: {
                var regionKey = BingIO.get_region_key(countries.currentIndex)
                switch (countries.currentIndex) {
                case 7:
                    countryLogo.source = "resources/china.png"
                    break
                case 1:
                    countryLogo.source = "resources/uk.png"
                    break
                case 2:
                    countryLogo.source = "resources/germany.png"
                    break
                case 3:
                    countryLogo.source = "resources/canada.png"
                    break
                case 4:
                    countryLogo.source = "resources/australia.png"
                    break
                case 5:
                    countryLogo.source = "resources/france.png"
                    break
                case 6:
                    countryLogo.source = "resources/japan.png"
                    break
                default:
                    countryLogo.source = "resources/us.png"
                }
                BingIO.set_region(regionKey)
            }

        }

        PreferencesPage {
            id: preferencesPage
            property alias preferencesPage: preferencesPage
            fiveRadionBtn.onClicked: {
                setTimer()            }

            tenRadioBtn.onClicked: {
                setTimer()
            }

            fifteenRadionBtn.onClicked: {
                setTimer()
            }

            thirtyRadionBtn.onClicked: {
                setTimer()
            }

            sixtyRadionBtn.onClicked: {
                setTimer()
            }

            customRadionBtn.onClicked: {
                openPopup()
            }

            downloadFolderComboBox.onCurrentTextChanged: {
                BingIO.set_bing_wall_directory(downloadFolderComboBox.currentText)
            }
            rotateImagesSwitch.onCheckedChanged: {
                rotateGroupBox.enabled = !rotateGroupBox.enabled
                if (rotateImagesSwitch.checked) {
                    setTimer()
                }
                else {
                    timer1.running = false
                    rotateButtonGroup.checkedButton.checked = false
                    preferencesPage.customIntervalText.visible = false
                    BingIO.set_rotate(0)
                }
            }

            Component.onCompleted: {
                var rotate = Number(BingIO.get_rotate)
                if (rotate > 0) {
                    rotateImagesSwitch.checked = true
                    setRotateCheckBoxes(rotate)
                    setTimer()
                }
            }

            downloadFolderComboBox.model: BingIO.directories

            function setTimer(customTimer) {
                customIntervalText.text = ""
                var interval = 0
                if (fiveRadionBtn.checked) {
                    interval = 300000
                    timer1.interval = interval
                }else if (tenRadioBtn.checked) {
                    interval = 600000
                    timer1.interval = interval
                }else if (fifteenRadionBtn.checked) {
                    interval = 900000
                    timer1.interval = interval
                }else if (thirtyRadionBtn.checked) {
                    interval = 1800000
                    timer1.interval = interval
                }else if (sixtyRadionBtn.checked) {
                    interval = 3600000
                    timer1.interval = interval
                }else if (customRadionBtn.checked) {
                    interval = BingIO.get_rotate
                    customIntervalText.text = "Rotate images every: " + Math.round(interval / 60000) + "ms"
                    timer1.interval = interval
                }
                if (interval > 0) {
                    BingIO.set_rotate(interval)
                    timer1.running = true
                }
            }

            function setRotateCheckBoxes(rotate) {
                if (rotate === 300000) {
                    fiveRadionBtn.checked = true
                }else if (rotate === 600000) {
                    tenRadioBtn.checked = true
                }else if (rotate === 900000) {
                    fifteenRadionBtn.checked = true
                }else if (rotate === 1800000) {
                    thirtyRadionBtn.checked = true
                }else if (rotate === 3600000) {
                    sixtyRadionBtn.checked = true
                }else {
                    customIntervalText.text = "Rotate images every: " + Math.round(rotate / 60000) + "ms"
                    customRadionBtn.checked = true
                }
                timer1.interval = rotate
            }
        }

        AboutForm {

        }
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex

        TabButton {
            text: qsTr("Settings")
        }
        TabButton {
            text: qsTr("Preferences")
        }

        TabButton {
            text: qsTr("About")
        }
    }

    Dialog {
        id: customTimerInputDialog
        title: "Custom timer"
        height: 150
        width: 300
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onAccepted: {
            var interval = (Number(hoursColumn.currentIndex) * (60000 * 60)) + (Number(minutesColumn.currentIndex)*60000)
            if (interval < 180000 && hoursColumn.currentIndex === 0) {
                messageDialog.open()
            }else {
                timer1.interval = interval
                timer1.running = true
                BingIO.set_rotate(interval)
                preferencesPage.customIntervalText.visible = true
                preferencesPage.customIntervalText.text = "Rotate images every: " + Math.round(interval / 60000) + "ms"
            }
        }

        onRejected: {
            BingIO.set_rotate(0)
        }

        //focus: true    // Needed in 5.9+ or this code is NOT going to work!!

        Column {
            anchors.fill: parent
            Row {
                Rectangle {
                    width: 99
                    height: 30
                    color: "transparent"
                    Text {
                        id: hours
                        text: qsTr("Hours")
                        anchors.centerIn: parent
                    }
                }
                Rectangle {
                    width: 100
                    height: 30
                    color: "transparent"
                    Text {
                        id: minutes
                        text: qsTr("Minutes")
                        anchors.centerIn: parent
                    }
                }
            }

            Row {
                Tumbler {
                    id: tumbler1
                    height: 60
                    TumblerColumn {
                        model: 10
                        id: hoursColumn
                    }

                    TumblerColumn {
                        model: 60
                        id: minutesColumn
                    }
                }
            }
        }
    }

    MessageDialog {
        id: messageDialog
        title: "Interval too short"
        text: "Please enter time greater than 2 minutes."
        onAccepted: {
            openPopup()
        }
    }

    function openPopup() {
        customTimerInputDialog.open()
    }
}

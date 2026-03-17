import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: scanPage
    background: Rectangle { color: "transparent" }

    required property var manager

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Item { Layout.preferredHeight: 20 }

        Text {
            text: "OpenBMS"
            font.pixelSize: 36
            font.bold: true
            font.family: "Segoe UI"
            color: "#00d4aa"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "JK BMS Monitor"
            font.pixelSize: 16
            font.family: "Segoe UI"
            color: "#8892b0"
            Layout.alignment: Qt.AlignHCenter
        }

        Item { Layout.preferredHeight: 4 }

        Text {
            text: scanPage.manager.statusText
            font.pixelSize: 13
            font.family: "Segoe UI"
            color: "#8892b0"
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            radius: 12
            color: scanPage.manager.scanning ? "#ff6b6b" : "#00d4aa"
            opacity: scanBtn.pressed ? 0.7 : (scanBtn.hovered ? 0.9 : 1.0)

            Behavior on color { ColorAnimation { duration: 200 } }
            Behavior on opacity { NumberAnimation { duration: 100 } }

            Text {
                anchors.centerIn: parent
                text: scanPage.manager.scanning ? "Stop Scan" : "Scan for Devices"
                color: "#0a0e1a"
                font.pixelSize: 15
                font.bold: true
                font.family: "Segoe UI"
            }

            MouseArea {
                id: scanBtn
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (scanPage.manager.scanning)
                        scanPage.manager.stopScan()
                    else
                        scanPage.manager.startScan()
                }
            }
        }

        ListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: scanPage.manager.devices

            delegate: Rectangle {
                width: deviceList.width
                height: 68
                radius: 12
                color: delegateMouse.containsMouse ? "#252538" : "#141927"

                Behavior on color { ColorAnimation { duration: 150 } }

                MouseArea {
                    id: delegateMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: scanPage.manager.connectToDevice(index)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#1a3a3a"

                        Text {
                            anchors.centerIn: parent
                            text: "BT"
                            color: "#00d4aa"
                            font.pixelSize: 13
                            font.bold: true
                            font.family: "Segoe UI"
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        Text {
                            text: modelData.name
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            font.family: "Segoe UI"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            text: modelData.address
                            color: "#5a6580"
                            font.pixelSize: 11
                            font.family: "Consolas"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        text: modelData.rssi + " dBm"
                        color: "#5a6580"
                        font.pixelSize: 11
                        font.family: "Segoe UI"
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: scanPage.manager.scanning
                      ? "Searching for devices..."
                      : "Tap 'Scan' to find BMS devices"
                color: "#3a4560"
                font.pixelSize: 14
                font.family: "Segoe UI"
                visible: deviceList.count === 0
            }
        }

        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: scanPage.manager.scanning
            visible: scanPage.manager.scanning
            palette.dark: "#00d4aa"
        }
    }
}

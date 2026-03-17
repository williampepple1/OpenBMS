import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: dashPage
    background: Rectangle { color: "transparent" }

    required property var manager

    property var bmsData: manager.data

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: mainColumn.implicitHeight
        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ColumnLayout {
            id: mainColumn
            width: parent.width
            spacing: 14

            // ── Header ──
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 16

                Text {
                    text: dashPage.bmsData.deviceName || "JK BMS"
                    color: "#ffffff"
                    font.pixelSize: 20
                    font.bold: true
                    font.family: "Segoe UI"
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: disconnectText.implicitWidth + 20
                    height: 32
                    radius: 8
                    color: disconnectMouse.containsMouse ? "#2a1525" : "transparent"
                    border.color: "#ff6b6b"
                    border.width: 1

                    Text {
                        id: disconnectText
                        anchors.centerIn: parent
                        text: "Disconnect"
                        color: "#ff6b6b"
                        font.pixelSize: 12
                        font.family: "Segoe UI"
                    }

                    MouseArea {
                        id: disconnectMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: dashPage.manager.disconnectDevice()
                    }
                }
            }

            // ── Status ──
            Text {
                text: dashPage.manager.statusText
                color: "#4a5578"
                font.pixelSize: 11
                font.family: "Segoe UI"
                Layout.alignment: Qt.AlignHCenter
            }

            // ── SOC Gauge ──
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 180

                Canvas {
                    id: socGauge
                    anchors.centerIn: parent
                    width: 160
                    height: 160

                    property int soc: dashPage.bmsData.soc

                    onSocChanged: requestPaint()
                    Component.onCompleted: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();

                        var cx = width / 2, cy = height / 2;
                        var r = 68, lw = 10;

                        ctx.beginPath();
                        ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                        ctx.strokeStyle = "#1e2538";
                        ctx.lineWidth = lw;
                        ctx.stroke();

                        if (soc > 0) {
                            var start = -Math.PI / 2;
                            var end = start + 2 * Math.PI * soc / 100;
                            ctx.beginPath();
                            ctx.arc(cx, cy, r, start, end);
                            ctx.strokeStyle = soc > 20 ? "#00d4aa"
                                            : soc > 10 ? "#ffb347" : "#ff6b6b";
                            ctx.lineWidth = lw;
                            ctx.lineCap = "round";
                            ctx.stroke();
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            text: socGauge.soc + "%"
                            color: "#ffffff"
                            font.pixelSize: 42
                            font.bold: true
                            font.family: "Segoe UI"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "State of Charge"
                            color: "#8892b0"
                            font.pixelSize: 11
                            font.family: "Segoe UI"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }

            // ── Stats Row ──
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                spacing: 10

                StatCard {
                    title: "Voltage"
                    value: dashPage.bmsData.totalVoltage.toFixed(2)
                    unit: "V"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                }

                StatCard {
                    title: "Current"
                    value: dashPage.bmsData.current.toFixed(2)
                    unit: "A"
                    accentColor: dashPage.bmsData.current >= 0 ? "#4ecdc4" : "#ff6b6b"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                }

                StatCard {
                    title: "Power"
                    value: dashPage.bmsData.power.toFixed(1)
                    unit: "W"
                    accentColor: "#a78bfa"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                }
            }

            // ── Cell Voltages ──
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                implicitHeight: cellCol.implicitHeight + 32
                radius: 16
                color: "#141927"

                ColumnLayout {
                    id: cellCol
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: "Cell Voltages"
                            color: "#ffffff"
                            font.pixelSize: 15
                            font.bold: true
                            font.family: "Segoe UI"
                            Layout.fillWidth: true
                        }

                        Text {
                            text: dashPage.bmsData.cellCount + "S"
                            color: "#8892b0"
                            font.pixelSize: 13
                            font.family: "Segoe UI"
                        }
                    }

                    Item { Layout.preferredHeight: 4 }

                    Repeater {
                        model: dashPage.bmsData.cellVoltages

                        CellBar {
                            Layout.fillWidth: true
                            cellNumber: index + 1
                            voltage: modelData
                            avgVoltage: dashPage.bmsData.avgCellVoltage
                            minVoltage: dashPage.bmsData.minCellVoltage
                            maxVoltage: dashPage.bmsData.maxCellVoltage
                        }
                    }

                    Item { Layout.preferredHeight: 4 }

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: "Min: " + dashPage.bmsData.minCellVoltage.toFixed(3) + "V"
                            color: "#8892b0"
                            font.pixelSize: 11
                            font.family: "Segoe UI"
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: "\u0394 " + (dashPage.bmsData.cellDelta * 1000).toFixed(0) + "mV"
                            color: dashPage.bmsData.cellDelta < 0.010 ? "#00d4aa"
                                 : dashPage.bmsData.cellDelta < 0.030 ? "#ffb347" : "#ff6b6b"
                            font.pixelSize: 11
                            font.bold: true
                            font.family: "Segoe UI"
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: "Max: " + dashPage.bmsData.maxCellVoltage.toFixed(3) + "V"
                            color: "#8892b0"
                            font.pixelSize: 11
                            font.family: "Segoe UI"
                        }
                    }

                    Text {
                        visible: dashPage.bmsData.cellCount === 0
                        text: "Waiting for cell data..."
                        color: "#3a4560"
                        font.pixelSize: 13
                        font.family: "Segoe UI"
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 20
                        Layout.bottomMargin: 20
                    }
                }
            }

            // ── Temperatures ──
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                implicitHeight: tempCol.implicitHeight + 32
                radius: 16
                color: "#141927"

                ColumnLayout {
                    id: tempCol
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: "Temperatures"
                        color: "#ffffff"
                        font.pixelSize: 15
                        font.bold: true
                        font.family: "Segoe UI"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: dashPage.bmsData.mosfetTemp.toFixed(1) + "°C"
                                color: dashPage.bmsData.mosfetTemp > 60 ? "#ff6b6b" : "#ffffff"
                                font.pixelSize: 18
                                font.bold: true
                                font.family: "Segoe UI"
                            }
                            Text {
                                text: "MOSFET"
                                color: "#8892b0"
                                font.pixelSize: 11
                                font.family: "Segoe UI"
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: dashPage.bmsData.temp1.toFixed(1) + "°C"
                                color: dashPage.bmsData.temp1 > 45 ? "#ff6b6b" : "#ffffff"
                                font.pixelSize: 18
                                font.bold: true
                                font.family: "Segoe UI"
                            }
                            Text {
                                text: "Battery 1"
                                color: "#8892b0"
                                font.pixelSize: 11
                                font.family: "Segoe UI"
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: dashPage.bmsData.temp2.toFixed(1) + "°C"
                                color: dashPage.bmsData.temp2 > 45 ? "#ff6b6b" : "#ffffff"
                                font.pixelSize: 18
                                font.bold: true
                                font.family: "Segoe UI"
                            }
                            Text {
                                text: "Battery 2"
                                color: "#8892b0"
                                font.pixelSize: 11
                                font.family: "Segoe UI"
                            }
                        }
                    }
                }
            }

            // ── Status Indicators ──
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                implicitHeight: statusRow.implicitHeight + 32
                radius: 16
                color: "#141927"

                RowLayout {
                    id: statusRow
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 20

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 10; height: 10; radius: 5
                            color: dashPage.bmsData.chargingEnabled ? "#00d4aa" : "#3a4560"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Charging"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.family: "Segoe UI"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 10; height: 10; radius: 5
                            color: dashPage.bmsData.dischargingEnabled ? "#00d4aa" : "#3a4560"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Discharging"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.family: "Segoe UI"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 10; height: 10; radius: 5
                            color: dashPage.bmsData.balancingActive ? "#ffb347" : "#3a4560"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Balancing"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.family: "Segoe UI"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // ── Additional Info ──
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                implicitHeight: infoCol.implicitHeight + 32
                radius: 16
                color: "#141927"

                ColumnLayout {
                    id: infoCol
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: "Battery Info"
                        color: "#ffffff"
                        font.pixelSize: 15
                        font.bold: true
                        font.family: "Segoe UI"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 8

                        Text { text: "Cycle Count"; color: "#8892b0"; font.pixelSize: 12; font.family: "Segoe UI" }
                        Text {
                            text: dashPage.bmsData.cycleCount.toString()
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Segoe UI"
                            Layout.alignment: Qt.AlignRight
                        }

                        Text { text: "Capacity"; color: "#8892b0"; font.pixelSize: 12; font.family: "Segoe UI" }
                        Text {
                            text: dashPage.bmsData.capacityRemaining.toFixed(1) + " / "
                                  + dashPage.bmsData.nominalCapacity.toFixed(1) + " Ah"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Segoe UI"
                            Layout.alignment: Qt.AlignRight
                        }

                        Text { text: "Avg Cell"; color: "#8892b0"; font.pixelSize: 12; font.family: "Segoe UI" }
                        Text {
                            text: dashPage.bmsData.avgCellVoltage.toFixed(3) + " V"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Segoe UI"
                            Layout.alignment: Qt.AlignRight
                        }

                        Text { text: "Cell Delta"; color: "#8892b0"; font.pixelSize: 12; font.family: "Segoe UI" }
                        Text {
                            text: (dashPage.bmsData.cellDelta * 1000).toFixed(1) + " mV"
                            color: dashPage.bmsData.cellDelta < 0.010 ? "#00d4aa"
                                 : dashPage.bmsData.cellDelta < 0.030 ? "#ffb347" : "#ff6b6b"
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Segoe UI"
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }
}

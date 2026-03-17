import QtQuick
import QtQuick.Layouts

Item {
    id: root
    height: 28

    required property int cellNumber
    required property double voltage
    required property double avgVoltage
    required property double minVoltage
    required property double maxVoltage

    property double deviationMv: Math.abs(voltage - avgVoltage) * 1000
    property color barColor: deviationMv < 5 ? "#00d4aa"
                           : deviationMv < 15 ? "#ffb347" : "#ff6b6b"

    property double range: Math.max(maxVoltage - minVoltage, 0.005)
    property double fillPercent: {
        var normalized = (voltage - minVoltage) / range;
        return Math.max(0.15, Math.min(1.0, 0.15 + 0.85 * normalized));
    }

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            text: root.cellNumber < 10 ? " C" + root.cellNumber : "C" + root.cellNumber
            color: "#8892b0"
            font.pixelSize: 11
            font.bold: true
            font.family: "Consolas"
            Layout.preferredWidth: 26
        }

        Rectangle {
            Layout.fillWidth: true
            height: 18
            radius: 4
            color: "#1e2538"

            Rectangle {
                width: parent.width * root.fillPercent
                height: parent.height
                radius: 4
                color: root.barColor
                opacity: 0.8

                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }
            }
        }

        Text {
            text: root.voltage.toFixed(3) + "V"
            color: "#ffffff"
            font.pixelSize: 11
            font.family: "Consolas"
            Layout.preferredWidth: 52
            horizontalAlignment: Text.AlignRight
        }
    }
}

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: card
    radius: 12
    color: "#141927"

    required property string title
    required property string value
    property string unit: ""
    property color accentColor: "#00d4aa"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 4

        Text {
            text: card.value + (card.unit ? " " + card.unit : "")
            color: card.accentColor
            font.pixelSize: 20
            font.bold: true
            font.family: "Segoe UI"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: card.title
            color: "#8892b0"
            font.pixelSize: 11
            font.family: "Segoe UI"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}

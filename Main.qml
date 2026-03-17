import QtQuick
import QtQuick.Controls
import OpenBMS

ApplicationWindow {
    id: root
    width: 420
    height: 750
    visible: true
    title: "OpenBMS"
    color: "#0a0e1a"

    BleManager {
        id: bms
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: scanPageComp

        pushEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: stack.width
                to: 0
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -stack.width * 0.3
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: -stack.width * 0.3
                to: 0
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: stack.width
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
    }

    Component {
        id: scanPageComp
        ScanPage {
            manager: bms
        }
    }

    Component {
        id: dashboardComp
        DashboardPage {
            manager: bms
        }
    }

    Connections {
        target: bms
        function onConnectedChanged() {
            if (bms.connected)
                stack.push(dashboardComp)
            else if (stack.depth > 1)
                stack.pop()
        }
    }
}

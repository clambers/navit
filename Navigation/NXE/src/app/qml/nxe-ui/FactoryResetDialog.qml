import QtQuick 2.0
import QtQuick.Layouts 1.1

Page {

    NDialog {
        id: dialogItem
        headerText: "Are you sure?"
        dialogText: "All saved maps, locations, destinations and settings will be removed"
        bottomBarComponent: RowLayout {
            width: dialogItem.width
            height: 60
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 5

            NButton {
                width: 100
                text: "Cancel"
                iconSource: "next_icon_white.png"
                onClicked: rootStack.pop({
                                             immediate: true
                                         })
            }

            Item {
                height: parent.height
                width: 300
                Layout.fillWidth: true
            }

            NButton {
                width: 190
                text: "Factory Reset Now"
                iconSource: "next_icon_white.png"
                onClicked: {
                    // we need to reset everything
                    // set ftu to true
                    // and pop to the main page
                    rootStack.push({
                                       item: Qt.resolvedUrl("SettingsView.qml"),
                                       properties: {
                                           ftu: true
                                       }
                                   })
                }
            }
        }
    }
}

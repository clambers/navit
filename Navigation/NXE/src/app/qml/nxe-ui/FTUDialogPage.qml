import QtQuick 2.0
import QtQuick.Layouts 1.1

Page {
    width: 400
    height: 800

    property alias dialogText: mainTextItem.text
    property alias headerText: headerTextItem.text
    property alias leftButtonComponent: leftButton

    Rectangle {
        id: blueRectangle
        width: parent.width
        height: 300
        color: "#09bcdf"
        anchors.centerIn: parent
    }

    Column {
        width: parent.width
        height: blueRectangle.height + 32
        anchors.top: blueRectangle.top
        anchors.topMargin: -32
        spacing: 5

        Image {
            id: logoImage
            width: 64
            height: 64
            anchors.horizontalCenter: parent.horizontalCenter
            source: "download_hex_icon.png"
        }

        Text {
            id: headerTextItem
            font.pixelSize: 29
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
        }

        Item {
            id: spacer
            height: 20
            width: parent.width
        }

        Item {
            width: parent.width
            height: 122

            Text {
                id: mainTextItem
                anchors.margins: 10
                width: parent.width / 2
                anchors.centerIn: parent

                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 23
                wrapMode: Text.WordWrap
                color: "white"
            }
        }

        Item {
            id: buttonBar
            width: parent.width
            height: 60

            RowLayout {
                height: 60
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 5

                NButton {
                    id: leftButton
                    width: 80
                    text: "Cancel"
                    onClicked: rootStack.push(Qt.resolvedUrl(
                                                  "FTUCancelPrompt.qml"))
                    bold: true
                }

                Item {
                    height: parent.height
                    width: 300
                    Layout.fillWidth: true
                }

                NButton {
                    width: 190
                    text: "Download a map"
                    bold: true
                    iconSource: "next_icon_white.png"
                    onClicked: {
                        rootStack.push({
                                           item: Qt.resolvedUrl(
                                                     "SettingsView.qml"),
                                           properties: {
                                               ftu: true
                                           }
                                       })
                    }
                }
            }
        }
    }
}


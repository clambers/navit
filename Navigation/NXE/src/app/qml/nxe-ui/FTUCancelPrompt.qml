import QtQuick 2.0

Rectangle {
    id: rectangle1
    width: 400
    height: 800
    color: "black"

    Rectangle {
        id: blue
        width: parent.width
        height: 300
        color: "#09bcdf"
        anchors.centerIn: parent

        Item {
            id: info
            width: 200
            height: 122
            anchors.centerIn: parent

            Text {
                anchors.fill: parent
                text: "Navit can't run without a map"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 23
                wrapMode: Text.WordWrap
                color: "white"
            }
        }


        Item {
            x: 195
            y: 230
            width: 157
            height: 60
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 48
            Item {
                width: 162
                anchors.fill: parent

                Text {
                    id: text
                    text: "Download a map"
                    font.pixelSize: 23
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"
                }

                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: text.right
                    anchors.leftMargin: 5
                    source: "next_icon_white.png"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        rootStack.push({item:Qt.resolvedUrl("SettingsView.qml"), properties: {ftu: true}})
                    }
                }
            }
        }

        Item {
            id: item1
            x: 33
            y: 240
            width: 112
            height: 40

            Text {
                id: text1
                y: 3
                color: "#ffffff"
                text: qsTr("Quit")
                font.pixelSize: 23
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            MouseArea {
                id: cancelMouseArea
                width: 112
                height: 40
                onClicked: navitProxy.quit()
            }
        }

        Image {
            id: logo
            x: 168
            y: -34
            width: 64
            height: 64
            anchors.horizontalCenterOffset: 0
            anchors.top: parent.top
            anchors.topMargin: -32
            anchors.horizontalCenter: parent.horizontalCenter
            source: "alert_hex_icon.png"
        }

        Text {
            x: 100
            text: "Are you sure?"
            anchors.top: logo.bottom
            anchors.topMargin: 5
            anchors.horizontalCenterOffset: 0
            font.pixelSize: 29
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
     }
    }

}



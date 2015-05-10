import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

Item {
    id: root
    property bool bold
    property bool _iconVisible: iconSource !== ''
    property alias text: buttonText.text
    property alias iconSource: imageItem.source
    property bool iconVisible: iconSource.toString().length !== 0
    property alias iconWidth: imageItem.sourceSize.width
    property alias iconHeight: imageItem.sourceSize.height
    property alias layout: rowLayout.layoutDirection

    width: 100
    height: 60

    signal clicked

    Component.onCompleted:  console.debug(iconSource)

    Rectangle {
        visible: false
        anchors.fill: parent
        color: "green"
    }

    Item {
        anchors.fill: parent

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            spacing: text.length !== 0 ? 5 : 0

            Text {
                id: buttonText
                font.bold: root.bold
                color: "white"
                font.pixelSize: 18
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                height: parent.height
            }

            Rectangle {
                width: iconVisible ? 32 : 0
                height: parent.height
                color: "transparent"
                visible: iconVisible
                Component.onCompleted: console.debug(imageItem.source, visible, width)

                Image {
                    id: imageItem
                    anchors.centerIn: parent
                }
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.clicked()
        }
    }
}

import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import sumdMonitor 1.0

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("sumdMonitor")

    BackEnd {
        id: backend

        onFrameChanged: {
            var dv = new DataView(frame)
            var chSize = dv.getUint8(2)

            for (var i=0; i < chSize; i++)
            {
                if (channelViewModel.count < chSize)
                {
                    channelViewModel.append({"name": i+1, "min": 0x2260, "max": 0x3b60, "value": dv.getUint16((i+1)*2+1)})
                }
                else
                {
                    channelViewModel.set(i, {"name": i+1, "min": 0x2260, "max": 0x3b60, "value": dv.getUint16((i+1)*2+1)})
                }
            }
        }

        onCrcErrorCountChanged: {
            crcText.text = "CRC Errors: " + crcErrorCount
        }
    }

    ComboBox {
        id: control
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8

        model: backend.ports

        delegate: ItemDelegate {
            width: control.width
            contentItem: Text {
                text: modelData
                color: "black"
                font: control.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: control.highlightedIndex === index
        }

        contentItem: Text {
            leftPadding: 8
            rightPadding: control.indicator.width + control.spacing

            text: control.displayText
            font: control.font
            color: control.pressed ? "gray" : "black"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 40
            border.color: control.pressed ? "gray" : "black"
            border.width: control.visualFocus ? 2 : 1
            radius: 2
        }

        popup: Popup {
            y: control.height - 1
            width: control.width
            implicitHeight: contentItem.implicitHeight
            padding: 1

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: control.popup.visible ? control.delegateModel : null
                currentIndex: control.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "black"
                radius: 2
            }
        }

        onCurrentIndexChanged: backend.onPortSelectionChanged(currentIndex)
    }

    Button {
        id: button
        text: qsTr("")
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: control.right
        anchors.leftMargin: 8

        width: 40
        height: 40

        icon.source: "refresh.png"

        background: Rectangle {
            border.color: button.down ? "gray" : "black"
            border.width: 1
        }

        onPressed: backend.onUpdatePortsPressed()
    }

    Text {
        id: crcText
        height: 40
        text: "CRC Errors: 0"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        anchors.rightMargin: 8
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.left: button.right
        anchors.top: parent.top
        anchors.topMargin: 8
    }

    Component {
        id: channelViewDelegate

        RowLayout {
            spacing: 8
            width: parent.width

            Text {
                text: name
                font.bold: true
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter

                Layout.preferredWidth: 20
            }

            ProgressBar
            {
                value: parseInt(text1.text, 10)
                from: min
                to: max

                Layout.fillWidth: true
            }

            Text {
                id: text1
                text: value
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter

                Layout.preferredWidth: 40
            }
        }
    }

    ScrollView{
        anchors.rightMargin: 8
        anchors.leftMargin: 8
        anchors.bottomMargin: 8
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: button.bottom

        ListView {
            id: listView
            orientation: ListView.Vertical
            anchors.fill: parent
            model: ListModel { id: channelViewModel }
            delegate: channelViewDelegate
        }

    }
}

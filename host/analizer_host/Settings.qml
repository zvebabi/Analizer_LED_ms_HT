import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    Connections {
        target: reciever
        onSendPortName: {
            availablePorts.append({"text": port});
            console.log("Received in QML from C++: " + port)
        }
    }
    Column {
        spacing: 10
        anchors.centerIn: parent

        Button {
            id: listDeviceBTN
            text: qsTr("ListDevice")
            width: 250
            onClicked: getPorts()
        }

        ComboBox{
            id: portsComboList
            objectName: "comboList"
            model: availablePorts
            width: 250
            ListModel{
                id: availablePorts
            }
        }

        ComboBox {
            id: baudRateComboList
            objectName: "baudRate"
            placeholderText: "choose baudRate"
            width: 250
        }

        Button {
            id: connectBTN
            text: qsTr("Connect")
            width: 250
            onClicked: initDevice(portsComboList.currentText)
        }

//        Text {
//            text: "Ответ:"
//            font.pixelSize: textArea.font.pixelSize
//        }

//        TextArea {
//            id: textArea
//            objectName: "textArea"
//            wrapMode: TextArea.Wrap
//            readOnly: true
//            width: 250
//        }
    }
}

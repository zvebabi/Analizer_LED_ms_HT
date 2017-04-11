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
            onClicked: {//getPorts()
                reciever.getListOfPort()
            }
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
            width: 250
            model: ["115200", "57600"]
        }

        Button {
            id: connectBTN
            text: qsTr("Connect")
            width: 250
            onClicked: reciever.initDevice(portsComboList.currentText,
                                  baudRateComboList.currentText)
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

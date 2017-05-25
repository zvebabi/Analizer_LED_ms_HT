import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import "QMLs"

Item {
    Connections {
        target: reciever
        onSendPortName: {
            availablePorts.append({"text": port});
            console.log(port)
        }
        onSendDebugIngo: {
            tipsWithPath.showedText = qsTr("data is: " + data)
            tipsWithPath.open()
        }
    }

    Column {
        spacing: 10
        anchors.centerIn: parent

        Button {
            id: listDeviceBTN
            contentItem: ButtonLabel {text: qsTr("ListDevice")}
            width: 250
            onClicked: {//getPorts()
                availablePorts.clear()
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
            contentItem: ButtonLabel {text: qsTr("Connect")}
            width: 250
            onClicked: reciever.initDevice(portsComboList.currentText,
                                  baudRateComboList.currentText)
        }
        CheckBox {
            id:drawSeries
            text: qsTr("Draw etalon data")
            checked: app.drawEt
            onClicked: {
                reciever.drawSeries(checked)
                app.drawEt = checked
            }
        }

//        Text {
//            text: "Ответ:"
//            font.pixelSize: textArea.font.pixelSize
//        }
//TODO: save path to directory with data&graphs
//        Rectangle {
//            TextField {
//                id: textArea
//                objectName: "textArea"
//                wrapMode: TextArea.Wrap
//                readOnly: true
//                width: 250
//            }
//            MouseArea
//            {
//               id: mouseArea
//               anchors.fill: parent
//               onClicked: {

//               }
//            }
//        }
    }
}
